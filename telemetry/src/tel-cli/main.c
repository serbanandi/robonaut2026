#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <zmq.h>
#include "commands.h"

// --- Main ---

void print_usage(const char* prog)
{
    printf("Usage: %s <command> [options]\n", prog);
    printf("Commands:\n");
    printf("  logs [-n <count>] [-f]      Show/Follow logs\n");
    printf("  ls                          List variables\n");
    printf("  show-vars                   Real-time variable monitor\n");
    printf("  read-var <name>             Monitor a variable\n");
    printf("  write-var <name> <val>      Set a variable\n");
    printf("  text <msg...>               Send text to server\n");
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        return 1;
    }

    void* ctx = zmq_ctx_new();
    const char* cmd = argv[1];

    if (strcmp(cmd, "logs") == 0)
    {
        // Pass remaining args
        cmd_logs(ctx, argc - 2, argv + 2);
    }
    else if (strcmp(cmd, "ls") == 0)
    {
        cmd_ls(ctx);
    }
    else if (strcmp(cmd, "show-vars") == 0)
    {
        cmd_show_vars(ctx);
    }
    else if (strcmp(cmd, "read-var") == 0)
    {
        if (argc < 3)
        {
            printf("Error: Missing variable name.\n");
        }
        else
        {
            cmd_read_var(ctx, argv[2]);
        }
    }
    else if (strcmp(cmd, "write-var") == 0)
    {
        if (argc < 4)
        {
            printf("Error: Usage: write-var <name> <value>\n");
        }
        else
        {
            cmd_write_var(ctx, argv[2], argv[3]);
        }
    }
    else if (strcmp(cmd, "text") == 0)
    {
        if (argc < 3)
        {
            printf("Error: Missing text.\n");
        }
        else
        {
            cmd_text(ctx, argc - 2, argv + 2);
        }
    }
    else
    {
        printf("Unknown command: %s\n", cmd);
        print_usage(argv[0]);
    }

    zmq_ctx_term(ctx);
    return 0;
}
