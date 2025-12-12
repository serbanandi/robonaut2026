#ifndef ZMQS_INTERFACE_H
#define ZMQS_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>
#include "var.h"

typedef void (*zmqs_VarListGetter_t)(var_VarRegistryEntry_t**, uint32_t*);
typedef bool (*zmqs_VarValueUpdateHandler_t)(const char*, const char*);
typedef bool (*zmqs_SendTextInputHandler_t)(const char*);

typedef struct
{
    zmqs_VarListGetter_t varListGetter;
    zmqs_VarValueUpdateHandler_t onVarValueUpdate;
    zmqs_SendTextInputHandler_t onTextInput;
} zmqs_Callbacks_t;

typedef enum
{
    ZMQ_LOG_LEVEL_DEBUG = 0,
    ZMQ_LOG_LEVEL_INFO,
    ZMQ_LOG_LEVEL_WARN,
    ZMQ_LOG_LEVEL_ERROR
} zmqs_LogLevel_t;

void* zmqs_Init(const char* pubAddress, const char* repAddress);

void zmqs_HandleRequests(zmqs_Callbacks_t callbacks);

void zmqs_SendLogMessage(zmqs_LogLevel_t level, uint32_t timeStamp, const char* message);

void zmqs_SendVarValueUpdate(var_VarRegistryEntry_t var);

void zmqs_Close();

#endif // ZMQS_INTERFACE_H
