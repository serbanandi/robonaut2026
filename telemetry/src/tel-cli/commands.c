#include "commands.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <zmq.h>

#define ZMQ_PUB_ADDR "tcp://localhost:5555"
#define ZMQ_REQ_ADDR "tcp://localhost:5556"
#define RCV_TIMEOUT_MS 500
#define MAX_TRACKED_VARS 100

typedef struct
{
    char name[50];
    char value[20];
    int type;
    int writeable;
} LocalVar_t;

static const char* get_log_level_str(uint8_t level)
{
    switch (level)
    {
        case 0: return "DEBUG";
        case 1: return "INFO ";
        case 2: return "WARN ";
        case 3: return "ERROR";
        default: return "UNK  ";
    }
}

// Helper to open a REQ socket, send a string, and get a reply
// Returns the reply size, or -1 on error.
// The caller must free 'reply_buffer' if successful.
static int simple_req(void* ctx, const char* req_str, char** reply_buffer)
{
    void* socket = zmq_socket(ctx, ZMQ_REQ);
    if (!socket)
        return -1;

    // Set Timeout
    int timeout = RCV_TIMEOUT_MS;
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_setsockopt(socket, ZMQ_LINGER, &timeout, sizeof(timeout));

    if (zmq_connect(socket, ZMQ_REQ_ADDR) != 0)
    {
        zmq_close(socket);
        return -1;
    }

    if (zmq_send(socket, req_str, strlen(req_str), 0) == -1)
    {
        perror("Failed to send request");
        zmq_close(socket);
        return -1;
    }

    zmq_msg_t msg;
    zmq_msg_init(&msg);
    if (zmq_msg_recv(&msg, socket, 0) == -1)
    {
        fprintf(stderr, "Timeout waiting for server response.\n");
        zmq_close(socket);
        zmq_msg_close(&msg);
        return -1;
    }

    size_t size = zmq_msg_size(&msg);
    *reply_buffer = malloc(size + 1);
    memcpy(*reply_buffer, zmq_msg_data(&msg), size);
    (*reply_buffer)[size] = '\0'; // Null terminate for safety (though binary might contain \0)

    zmq_msg_close(&msg);
    zmq_close(socket);
    return (int) size;
}

int cmd_logs(void* ctx, int argc, char* argv[])
{
    int n = 10;
    int follow = 0;

    // 1. Argument Parsing
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc)
        {
            n = atoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-f") == 0)
        {
            follow = 1;
        }
    }

    // 2. Fetch History (REQ-REP)
    char req_str[32];
    snprintf(req_str, sizeof(req_str), "LOGS %d", n);

    char* resp_buf = NULL;
    int resp_len = simple_req(ctx, req_str, &resp_buf);

    if (resp_len > 0)
    {
        // Format: "LOGS <count>\0{BINARY_BLOB}"
        // The text part ends at the first null terminator.

        // Find the separator between the text header and binary data
        char* separator = memchr(resp_buf, '\0', resp_len);

        if (separator && strncmp(resp_buf, "LOGS ", 5) == 0)
        {
            int count = atoi(resp_buf + 5);

            // Pointer to the start of the binary blob (byte after \0)
            uint8_t* ptr = (uint8_t*) (separator + 1);
            uint8_t* end = (uint8_t*) (resp_buf + resp_len);

            for (int i = 0; i < count; i++)
            {
                // Check if we have enough bytes for TS(4) + LVL(1)
                if (ptr + 5 > end)
                    break;

                // 1. Timestamp (4 bytes)
                uint32_t ts;
                memcpy(&ts, ptr, 4);
                ptr += 4;

                // 2. Log Level (1 byte)
                uint8_t lvl = *ptr;
                ptr += 1;

                // 3. Message (Null-terminated string)
                // Verify the string is actually terminated within the buffer bounds
                char* msg_start = (char*) ptr;
                char* msg_end = memchr(msg_start, '\0', end - ptr);

                if (!msg_end)
                {
                    printf("Error: Malformed log message (missing null terminator)\n");
                    break;
                }

                printf("[%u] [%s] %s\n", ts, get_log_level_str(lvl), msg_start);

                // Advance pointer past the message and its null terminator
                ptr = (uint8_t*) (msg_end + 1);
            }
        }
        else
        {
            printf("Invalid response format.\n");
        }
        free(resp_buf);
    }

    // 3. Follow (SUB)
    if (follow)
    {
        printf("--- Following Logs ---\n");
        void* sub = zmq_socket(ctx, ZMQ_SUB);
        zmq_connect(sub, ZMQ_PUB_ADDR);
        zmq_setsockopt(sub, ZMQ_SUBSCRIBE, "LOG", 3);

        while (1)
        {
            char buf[1024];
            int len = zmq_recv(sub, buf, sizeof(buf) - 1, 0);
            if (len > 0)
            {
                buf[len] = '\0';
                // Format: LOG <ts> <lvl> <msg>
                uint32_t ts;
                int lvl_int;
                // Scanf trick: %*s skips "LOG", %[^\n] reads the rest of the line
                // But we need to skip "LOG " manually to be safe or use offset
                if (strncmp(buf, "LOG ", 4) == 0)
                {
                    char msg[512];
                    if (sscanf(buf + 4, "%u %d %[^\n]", &ts, &lvl_int, msg) == 3)
                    {
                        printf("[%u] [%s] %s\n", ts, get_log_level_str((uint8_t) lvl_int), msg);
                    }
                    else
                    {
                        printf("%s\n", buf); // Fallback
                    }
                }
            }
        }
        zmq_close(sub);
    }
    return 0;
}

int cmd_show_vars(void* ctx)
{
    LocalVar_t vars[MAX_TRACKED_VARS];
    int var_count = 0;
    memset(vars, 0, sizeof(vars));

    // 1. Initial Discovery: Get the list of variables
    // Note: VARLIST only gives Name/Type/Access, NOT the value.
    // We will initialize values to "?" until an update arrives.
    char* resp = NULL;
    int len = simple_req(ctx, "VARLIST", &resp);

    if (len > 0 && strncmp(resp, "VARLIST", 7) == 0)
    {
        int n = 0;
        int offset = 0;
        sscanf(resp, "VARLIST %d%n", &n, &offset);
        char* ptr = resp + offset;

        for (int i = 0; i < n && var_count < MAX_TRACKED_VARS; i++)
        {
            int read_chars = 0;
            if (sscanf(ptr, " %49s %d %d%n", vars[var_count].name, &vars[var_count].type, &vars[var_count].writeable,
                       &read_chars) == 3)
            {
                strcpy(vars[var_count].value, "?"); // Unknown initial value
                var_count++;
                ptr += read_chars;
            }
            else
            {
                break;
            }
        }
    }
    if (resp)
        free(resp);

    // 2. Subscribe to Updates
    void* sub = zmq_socket(ctx, ZMQ_SUB);
    zmq_connect(sub, ZMQ_PUB_ADDR);
    // Subscribe to all variable updates
    zmq_setsockopt(sub, ZMQ_SUBSCRIBE, "VAR", 3);

    // 3. Render Loop
    printf("\033[2J"); // Clear Screen once at start

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t last_render_ms = 0;

    while (1)
    {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint64_t now_ms = (uint64_t) ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

        // --- Render (throttled to 100ms) ---
        if (now_ms - last_render_ms >= 100)
        {
            printf("\033[H"); // Move Cursor to Home (Top-Left)
            printf("--- Real-time Variable Monitor (Ctrl+C to exit) ---\n\n");
            printf("%-50s %-8s %-6s %-20s\n", "Name", "Type", "Access", "Current Value");
            printf("----------------------------------------------------------------------\n");

            for (int i = 0; i < var_count; i++)
            {
                const char* type_str = "UNK";
                switch (vars[i].type)
                {
                    case 0: type_str = "u8"; break;
                    case 1: type_str = "i8"; break;
                    case 2: type_str = "u16"; break;
                    case 3: type_str = "i16"; break;
                    case 4: type_str = "u32"; break;
                    case 5: type_str = "i32"; break;
                    case 6: type_str = "flt"; break;
                }

                // Print the row
                // \033[K clears the rest of the line (avoids artifacts when values shrink)
                printf("%-50s %-8s %-6s %-20s\033[K\n", vars[i].name, type_str, vars[i].writeable ? "RW" : "RO",
                       vars[i].value);
            }
            printf("\033[J"); // Clear anything below this point
            fflush(stdout);
            last_render_ms = now_ms;
        }

        // --- Wait for Update ---
        // Calculate how long to wait until the next render slot
        long timeout = 100 - (long) (now_ms - last_render_ms);
        if (timeout < 0)
            timeout = 0;

        zmq_pollitem_t items[] = { { sub, 0, ZMQ_POLLIN, 0 } };
        int rc = zmq_poll(items, 1, timeout);

        if (rc > 0 && (items[0].revents & ZMQ_POLLIN))
        {
            char buf[256];
            int recv_len = zmq_recv(sub, buf, sizeof(buf) - 1, 0);

            if (recv_len > 0)
            {
                buf[recv_len] = '\0';
                // Parse: VAR <name> <val>
                char rx_name[50];
                char rx_val[20];

                if (sscanf(buf, "VAR %49s %19s", rx_name, rx_val) == 2)
                {
                    for (int i = 0; i < var_count; i++)
                    {
                        if (strcmp(vars[i].name, rx_name) == 0)
                        {
                            strncpy(vars[i].value, rx_val, 19);
                            break;
                        }
                    }
                }
            }
        }
    }

    zmq_close(sub);
    return 0;
}

int cmd_ls(void* ctx)
{
    char* resp = NULL;
    int len = simple_req(ctx, "VARLIST", &resp);

    if (len > 0)
    {
        // Format: VARLIST <N> <name> <type> <writeable> ...
        if (strncmp(resp, "VARLIST", 7) == 0)
        {
            int n = 0;
            int offset = 0;
            sscanf(resp, "VARLIST %d%n", &n, &offset);

            char* ptr = resp + offset;

            printf("%-50s %-10s %-10s\n", "Name", "Type", "Access");
            printf("----------------------------------------------------------------------\n");

            char name[50];
            int type, writeable;
            int read_chars;

            for (int i = 0; i < n; i++)
            {
                // Parse space separated tokens
                if (sscanf(ptr, " %s %d %d%n", name, &type, &writeable, &read_chars) == 3)
                {
                    const char* type_str = "UNK";
                    switch (type)
                    {
                        case 0: type_str = "u8"; break;
                        case 1: type_str = "i8"; break;
                        case 2: type_str = "u16"; break;
                        case 3: type_str = "i16"; break;
                        case 4: type_str = "u32"; break;
                        case 5: type_str = "i32"; break;
                        case 6: type_str = "flt"; break;
                    }
                    printf("%-50s %-10s %-10s\n", name, type_str, writeable ? "RW" : "RO");
                    ptr += read_chars;
                }
                else
                {
                    break;
                }
            }
        }
        free(resp);
    }
    return 0;
}

int cmd_read_var(void* ctx, const char* varname)
{
    void* sub = zmq_socket(ctx, ZMQ_SUB);
    zmq_connect(sub, ZMQ_PUB_ADDR);

    // Filter specifically for "VAR <varname> "
    char filter[64];
    snprintf(filter, sizeof(filter), "VAR %s", varname);
    zmq_setsockopt(sub, ZMQ_SUBSCRIBE, filter, strlen(filter));

    printf("Watching variable '%s'...\n", varname);

    while (1)
    {
        char buf[256];
        int len = zmq_recv(sub, buf, sizeof(buf) - 1, 0);
        if (len > 0)
        {
            buf[len] = '\0';
            // Output: VAR myVar 123
            // We strip the prefix to just show the value
            char name[50];
            char val[64];
            if (sscanf(buf, "VAR %49s %63s", name, val) == 2)
            {
                printf("%s = %s\n", name, val);
            }
            else
            {
                printf("%s\n", buf);
            }
        }
    }
    zmq_close(sub);
    return 0;
}

int cmd_write_var(void* ctx, const char* name, const char* val)
{
    char req[128];
    snprintf(req, sizeof(req), "SET %s %s", name, val);

    char* resp = NULL;
    int len = simple_req(ctx, req, &resp);

    if (len > 0)
    {
        printf("%s\n", resp);
        free(resp);
    }
    return 0;
}

int cmd_text(void* ctx, int argc, char* argv[])
{
    // Concatenate args starting from argv[0] (which is the first text word)
    char buffer[512];
    buffer[0] = '\0';

    strcat(buffer, "TEXT ");
    for (int i = 0; i < argc; i++)
    {
        strcat(buffer, argv[i]);
        if (i < argc - 1)
            strcat(buffer, " ");
    }

    char* resp = NULL;
    int len = simple_req(ctx, buffer, &resp);
    if (len > 0)
    {
        // usually just "OK" or nothing for text submission
        printf("Server: %s\n", resp);
        free(resp);
    }
    return 0;
}
