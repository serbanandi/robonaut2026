#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stdint.h>

#define PROTOCOL_SOF 42
#define PROTOCOL_EOF 69
#define PROTOCOL_ESC 123

#define UART_BUFFER_SIZE 4000

typedef enum
{
    UART_RX_STATE_WAIT_SOF = 0,
    UART_RX_STATE_READ,
    UART_RX_STATE_READ_ESCAPED
} _uart_RxState_t;

bool _uart_SendFull(const uint8_t* data, int32_t length);

#endif // UART_H