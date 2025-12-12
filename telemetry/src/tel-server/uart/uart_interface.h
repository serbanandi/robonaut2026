#ifndef UART_INTERFACE_H
#define UART_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

typedef void (*uart_ReceiveCallback_t)(const uint8_t*, int32_t);

int uart_Init(const char* device, int baudrate);

bool uart_Receive(uart_ReceiveCallback_t callback);

bool uart_Send(const uint8_t* data, int32_t length);

void uart_Close();

#endif // UART_H
