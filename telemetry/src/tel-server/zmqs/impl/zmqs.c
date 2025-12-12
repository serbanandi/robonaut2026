#include "zmqs.h"
#include "../zmqs_interface.h"

#include <log.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

static void* _zmqs_ctx = NULL;
static void* _zmqs_pub = NULL;
static void* _zmqs_rep = NULL;

static _zmqs_LogMessage_t _zmqs_logHistory[ZMQS_MAX_LOG_HISTORY];
static int32_t _zmqs_logHead = 0;
static int32_t _zmqs_logCount = 0;

static char _zmqs_tmpBuffer[ZMQS_MAX_MSG_SIZE];

void* zmqs_Init(const char* pubAddress, const char* repAddress)
{
    _zmqs_ctx = zmq_ctx_new();
    if (!_zmqs_ctx)
    {
        log_error("Context Creation Failed");
        return NULL;
    }

    _zmqs_pub = zmq_socket(_zmqs_ctx, ZMQ_PUB);
    if (!_zmqs_pub)
    {
        log_error("PUB Socket Creation Failed");
        zmq_ctx_destroy(_zmqs_ctx);
        return NULL;
    }

    _zmqs_rep = zmq_socket(_zmqs_ctx, ZMQ_REP);
    if (!_zmqs_rep)
    {
        log_error("REP Socket Creation Failed");
        zmq_close(_zmqs_pub);
        zmq_ctx_destroy(_zmqs_ctx);
        return NULL;
    }

    if (zmq_bind(_zmqs_pub, pubAddress) != 0)
    {
        log_error("PUB Socket Bind Failed");
        zmq_close(_zmqs_rep);
        zmq_close(_zmqs_pub);
        zmq_ctx_destroy(_zmqs_ctx);
        return NULL;
    }

    if (zmq_bind(_zmqs_rep, repAddress) != 0)
    {
        log_error("REP Socket Bind Failed");
        zmq_close(_zmqs_rep);
        zmq_close(_zmqs_pub);
        zmq_ctx_destroy(_zmqs_ctx);
        return NULL;
    }

    return _zmqs_rep;
}

void zmqs_Close()
{
    if (_zmqs_rep)
        zmq_close(_zmqs_rep);
    if (_zmqs_pub)
        zmq_close(_zmqs_pub);
    if (_zmqs_ctx)
        zmq_ctx_destroy(_zmqs_ctx);
    _zmqs_ctx = NULL;
    _zmqs_pub = NULL;
    _zmqs_rep = NULL;
}

void zmqs_HandleRequests(zmqs_Callbacks_t callbacks)
{
    while (1)
    {
        int size = zmq_recv(_zmqs_rep, _zmqs_tmpBuffer, ZMQS_MAX_MSG_SIZE - 1, ZMQ_DONTWAIT);
        if (size == -1)
        {
            return;
        }
        _zmqs_tmpBuffer[size] = '\0';
        const char* OK_MSG = "OK";

        // 1. SET Command: "SET <name> <val>"
        if (strncmp(_zmqs_tmpBuffer, "SET", 3) == 0)
        {
            char name[VAR_MAX_VARNAME_LENGTH], valStr[20];
            if (sscanf(_zmqs_tmpBuffer, "SET %9s %19s", name, valStr) == 2)
            {
                if (callbacks.onVarValueUpdate)
                {
                    if (callbacks.onVarValueUpdate(name, valStr))
                    {
                        zmq_send(_zmqs_rep, OK_MSG, strlen(OK_MSG), 0);
                    }
                    else
                    {
                        const char* ERR_MSG = "ERR Update Failed";
                        zmq_send(_zmqs_rep, ERR_MSG, strlen(ERR_MSG), 0);
                        log_warn("Variable Update Failed for %s", name);
                    }
                }
            }
            else
            {
                const char* ERR_MSG = "ERR Invalid SET Format";
                zmq_send(_zmqs_rep, ERR_MSG, strlen(ERR_MSG), 0);
                log_warn("Invalid SET Command Format");
            }
        }
        // 2. LOGS Command: "LOGS <n>"
        else if (strncmp(_zmqs_tmpBuffer, "LOGS", 4) == 0)
        {
            int32_t count = 10;
            sscanf(_zmqs_tmpBuffer, "LOGS %d", &count);
            if (count > _zmqs_logCount)
                count = _zmqs_logCount;
            if (count <= 0)
            {
                zmq_send(_zmqs_rep, "", 0, 0);
                continue;
            }
            void* sendDataBuffer = malloc(count * sizeof(_zmqs_LogMessage_t) + 30);
            if (!sendDataBuffer)
            {
                log_error("Failed to allocate memory for log messages");
                const char* ERR_MSG = "ERR Memory Allocation Failed";
                zmq_send(_zmqs_rep, ERR_MSG, strlen(ERR_MSG), 0);
                continue;
            }
            int32_t offset = sprintf(sendDataBuffer, "LOGS %d", count) + 1;
            int currentCount = 0;
            int currentIndex = (_zmqs_logHead - count + ZMQS_MAX_LOG_HISTORY) % ZMQS_MAX_LOG_HISTORY;
            while (currentCount < count)
            {
                _zmqs_LogMessage_t* logEntry = &_zmqs_logHistory[currentIndex];
                memcpy((char*) sendDataBuffer + offset, &logEntry->timestamp, 4);
                offset += 4;
                memcpy((char*) sendDataBuffer + offset, &logEntry->level, 1);
                offset += 1;
                int msgLen = strlen(logEntry->message) + 1;
                memcpy((char*) sendDataBuffer + offset, logEntry->message, msgLen);
                offset += msgLen;
                currentIndex = (currentIndex + 1) % ZMQS_MAX_LOG_HISTORY;
                currentCount++;
            }
            zmq_send(_zmqs_rep, sendDataBuffer, offset, 0);
            free(sendDataBuffer);
        }
        // 3. VARLIST Command: "VARLIST"
        else if (strncmp(_zmqs_tmpBuffer, "VARLIST", 7) == 0)
        {
            if (callbacks.varListGetter)
            {
                var_VarRegistryEntry_t* varList = NULL;
                uint32_t varCount = 0;
                callbacks.varListGetter(&varList, &varCount);
                if (varList)
                {
                    void* sendDataBuffer = malloc(varCount * (VAR_MAX_VARNAME_LENGTH + 10) + 20);
                    if (!sendDataBuffer)
                    {
                        log_error("Failed to allocate memory for variable list");
                        const char* ERR_MSG = "ERR Memory Allocation Failed";
                        zmq_send(_zmqs_rep, ERR_MSG, strlen(ERR_MSG), 0);
                        continue;
                    }
                    int32_t offset = sprintf(sendDataBuffer, "VARLIST %d ", varCount);
                    for (uint32_t i = 0; i < varCount; i++)
                    {
                        int32_t written = sprintf((char*) sendDataBuffer + offset, "%s %d %d ", varList[i].name,
                                                  varList[i].type, varList[i].writable ? 1 : 0);
                        offset += written;
                    }
                    zmq_send(_zmqs_rep, sendDataBuffer, offset, 0);
                    free(sendDataBuffer);
                }
                else
                {
                    const char* ERR_MSG = "ERR No Variables";
                    zmq_send(_zmqs_rep, ERR_MSG, strlen(ERR_MSG), 0);
                    log_warn("No Variables to Send in VARLIST");
                }
            }
        }
        else if (strncmp(_zmqs_tmpBuffer, "TEXT", 4) == 0)
        {
            const char* inputStr = _zmqs_tmpBuffer + 5; // Skip "TEXT "
            if (callbacks.onTextInput)
            {
                if (callbacks.onTextInput(inputStr))
                {
                    zmq_send(_zmqs_rep, OK_MSG, strlen(OK_MSG), 0);
                }
                else
                {
                    const char* ERR_MSG = "ERR Text Input Handling Failed";
                    zmq_send(_zmqs_rep, ERR_MSG, strlen(ERR_MSG), 0);
                    log_warn("Text Input Handling Failed");
                }
            }
            else
            {
                const char* ERR_MSG = "ERR No Text Input Handler";
                zmq_send(_zmqs_rep, ERR_MSG, strlen(ERR_MSG), 0);
                log_warn("No Text Input Handler Registered");
            }
        }
        else
        {
            const char* ERR_MSG = "ERR Unknown Cmd";
            zmq_send(_zmqs_rep, ERR_MSG, strlen(ERR_MSG), 0);
        }
    }
}

void zmqs_SendLogMessage(zmqs_LogLevel_t level, uint32_t timestampMs, const char* message)
{
    // Store in history
    _zmqs_LogMessage_t* logEntry = &_zmqs_logHistory[_zmqs_logHead];
    logEntry->timestamp = timestampMs;
    logEntry->level = level;
    strncpy(logEntry->message, message, ZMQS_MAX_INCOMING_LOG_SIZE - 1);
    logEntry->message[ZMQS_MAX_INCOMING_LOG_SIZE - 1] = '\0';

    _zmqs_logHead = (_zmqs_logHead + 1) % ZMQS_MAX_LOG_HISTORY;
    if (_zmqs_logCount < ZMQS_MAX_LOG_HISTORY)
        _zmqs_logCount++;

    snprintf(_zmqs_tmpBuffer, sizeof(_zmqs_tmpBuffer), "LOG %u %d %s", timestampMs, level, message);
    zmq_send(_zmqs_pub, _zmqs_tmpBuffer, strlen(_zmqs_tmpBuffer), 0);
    log_debug("Sent Log Message via PUB");
}

void zmqs_SendVarValueUpdate(var_VarRegistryEntry_t var)
{
    // Prepare ZMQ Message
    char valueStr[20];
    var_GetString(var.value, var.type, valueStr, sizeof(valueStr));
    snprintf(_zmqs_tmpBuffer, sizeof(_zmqs_tmpBuffer), "VAR %s %s", var.name, valueStr);

    // Send via ZMQ PUB
    zmq_send(_zmqs_pub, _zmqs_tmpBuffer, strlen(_zmqs_tmpBuffer), 0);
    log_debug("Sent Var Value Update for '%s' via PUB", var.name);
}
