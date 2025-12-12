#ifndef CTRL_INTERFACE_H
#define CTRL_INTERFACE_H

#include <stdint.h>
#include "uart/uart_interface.h"
#include "var.h"
#include "zmqs/zmqs_interface.h"

void ctrl_Init();

void ctrl_UartReceiveCallback(const uint8_t* data, int32_t length);

bool ctrl_VarValueUpdateHandler(const char* name, const char* value);

bool ctrl_SendTextInputHandler(const char* inputStr);

void ctrl_GetVarRegistry(var_VarRegistryEntry_t** registry, uint32_t* count);

void ctrl_PeriodicTask();

#endif // CTRL_INTERFACE_H
