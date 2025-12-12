#ifndef ZMQS_H
#define ZMQS_H

#include "../zmqs_interface.h"
#include "var.h"

#define ZMQS_MAX_MSG_SIZE 512

#define ZMQS_MAX_INCOMING_LOG_SIZE 256
#define ZMQS_MAX_LOG_HISTORY 400

typedef struct
{
    uint32_t timestamp;
    zmqs_LogLevel_t level;
    char message[ZMQS_MAX_INCOMING_LOG_SIZE];
} _zmqs_LogMessage_t;

#endif // ZMQS_H
