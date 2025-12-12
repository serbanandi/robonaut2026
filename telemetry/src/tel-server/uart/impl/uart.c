#include "uart.h"
#include "../uart_interface.h"

#include <errno.h>
#include <fcntl.h>
#include <log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static int _uart_uartFd = -1;

static uint8_t _uart_rxTmpBuffer[UART_BUFFER_SIZE];
static uint8_t _uart_rxBuffer[UART_BUFFER_SIZE];
static uint32_t _uart_rxBufferHead = 0;
static _uart_RxState_t _uart_rxState = UART_RX_STATE_WAIT_SOF;
static uint8_t _uart_rxChecksum = 0;

static uint8_t _uart_txBuffer[UART_BUFFER_SIZE];

int uart_Init(const char* device, int baudrate)
{
    _uart_rxBufferHead = 0;
    _uart_rxState = UART_RX_STATE_WAIT_SOF;
    _uart_uartFd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);

    if (_uart_uartFd < 0)
    {
        log_error("Error opening UART: %s", strerror(errno));
        return -1;
    }

    struct termios tty;
    if (tcgetattr(_uart_uartFd, &tty) != 0)
    {
        log_error("Error from tcgetattr: %s", strerror(errno));
        return -1;
    }

    // 2. Set Baudrate
    cfsetospeed(&tty, baudrate);
    cfsetispeed(&tty, baudrate);

    // 3. Configure 8N1 (8 Data, No Parity, 1 Stop)
    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (Most important for "N")
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (Most important for "1")
    tty.c_cflag &= ~CSIZE;  // Clear all bits that set the data size
    tty.c_cflag |= CS8;     // 8 bits per byte (Most important for "8")

    // 4. Disable Hardware Flow Control
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control

    // 5. Standard Settings
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag = 0;                        // Canonical mode off, echo off, signal chars off
    tty.c_oflag = 0;                        // Raw output (no remapping of newlines)
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &=
        ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    // 6. Read Settings
    // Since we use select(), we usually want read() to return immediately
    // with whatever is available.
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 0;

    // Apply settings
    if (tcsetattr(_uart_uartFd, TCSANOW, &tty) != 0)
    {
        log_error("Error from tcsetattr: %s", strerror(errno));
        return -1;
    }

    return _uart_uartFd;
}

bool uart_Receive(uart_ReceiveCallback_t callback)
{
    int32_t len = read(_uart_uartFd, _uart_rxTmpBuffer, sizeof(_uart_rxTmpBuffer));
    if (len > 0)
    {
        for (int32_t i = 0; i < len; i++)
        {
            uint8_t byte = _uart_rxTmpBuffer[i];
            switch (_uart_rxState)
            {
                case UART_RX_STATE_WAIT_SOF:
                    if (byte == PROTOCOL_SOF)
                    {
                        _uart_rxBufferHead = 0;
                        _uart_rxChecksum = 0;
                        _uart_rxState = UART_RX_STATE_READ;
                    }
                    break;

                case UART_RX_STATE_READ:
                    if (byte == PROTOCOL_SOF)
                    {
                        log_warn("Unexpected SOF");
                        _uart_rxBufferHead = 0;
                        _uart_rxChecksum = 0;
                    }
                    else if (byte == PROTOCOL_ESC)
                    {
                        _uart_rxState = UART_RX_STATE_READ_ESCAPED;
                    }
                    else if (byte == PROTOCOL_EOF)
                    {
                        if (_uart_rxBufferHead > 1)
                        {
                            _uart_rxBufferHead--;
                            int receivedChecksum = _uart_rxBuffer[_uart_rxBufferHead];
                            int calcChecksum = ((int) _uart_rxChecksum - receivedChecksum + 0x100) & 0xFF;
                            if (receivedChecksum == calcChecksum)
                            {
                                callback(_uart_rxBuffer, _uart_rxBufferHead);
                            }
                            else
                            {
                                log_warn("Checksum Mismatch");
                            }
                        }
                        else
                        {
                            log_warn("Frame Too Short");
                        }
                        _uart_rxState = UART_RX_STATE_WAIT_SOF;
                    }
                    else if (_uart_rxBufferHead < UART_BUFFER_SIZE)
                    {
                        _uart_rxBuffer[_uart_rxBufferHead++] = byte;
                        _uart_rxChecksum += byte;
                    }
                    else
                    {
                        log_warn("RX Buffer Overflow");
                        _uart_rxState = UART_RX_STATE_WAIT_SOF;
                    }
                    break;

                case UART_RX_STATE_READ_ESCAPED:
                    if (_uart_rxBufferHead < UART_BUFFER_SIZE)
                    {
                        switch (byte)
                        {
                            case 0x01:
                                _uart_rxBuffer[_uart_rxBufferHead++] = PROTOCOL_SOF;
                                _uart_rxChecksum += PROTOCOL_SOF;
                                _uart_rxState = UART_RX_STATE_READ;
                                break;
                            case 0x03:
                                _uart_rxBuffer[_uart_rxBufferHead++] = PROTOCOL_EOF;
                                _uart_rxChecksum += PROTOCOL_EOF;
                                _uart_rxState = UART_RX_STATE_READ;
                                break;
                            case 0x02:
                                _uart_rxBuffer[_uart_rxBufferHead++] = PROTOCOL_ESC;
                                _uart_rxChecksum += PROTOCOL_ESC;
                                _uart_rxState = UART_RX_STATE_READ;
                                break;
                            default:
                                log_warn("Invalid Escape Sequence");
                                _uart_rxState = UART_RX_STATE_WAIT_SOF;
                                break;
                        }
                    }
                    else
                    {
                        log_warn("RX Buffer Overflow");
                        _uart_rxState = UART_RX_STATE_WAIT_SOF;
                    }
                    break;

                default: _uart_rxState = UART_RX_STATE_WAIT_SOF; break;
            }
        }
        return true;
    }
    return false;
}

bool uart_Send(const uint8_t* data, int32_t length)
{
    int32_t txIndex = 0;
    uint8_t checksum = 0;

    // Start of Frame
    _uart_txBuffer[txIndex++] = PROTOCOL_SOF;

    // Data Encoding
    for (int32_t i = 0; i <= length; i++)
    {
        uint8_t byte = data[i];
        if (i == length)
            byte = checksum;
        else
            checksum += byte;
        if (byte == PROTOCOL_SOF)
        {
            _uart_txBuffer[txIndex++] = PROTOCOL_ESC;
            _uart_txBuffer[txIndex++] = 0x01;
        }
        else if (byte == PROTOCOL_ESC)
        {
            _uart_txBuffer[txIndex++] = PROTOCOL_ESC;
            _uart_txBuffer[txIndex++] = 0x02;
        }
        else if (byte == PROTOCOL_EOF)
        {
            _uart_txBuffer[txIndex++] = PROTOCOL_ESC;
            _uart_txBuffer[txIndex++] = 0x03;
        }
        else
        {
            _uart_txBuffer[txIndex++] = byte;
        }
    }

    // End of Frame
    _uart_txBuffer[txIndex++] = PROTOCOL_EOF;

    return _uart_SendFull(_uart_txBuffer, txIndex);
}

bool _uart_SendFull(const uint8_t* data, int32_t length)
{
    int32_t total_sent = 0;
    while (total_sent < length)
    {
        int32_t n = write(_uart_uartFd, data + total_sent, length - total_sent);
        if (n < 0)
        {
            // Check if the buffer is just full (EAGAIN/EWOULDBLOCK)
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // Buffer is full (likely hitting the 16-byte limit on WSL).
                // Sleep for a tiny amount to let the UART hardware drain.
                // 100 microseconds is enough for ~10 bytes to leave at 921600 baud.
                log_debug("Write would block, sleeping briefly");
                usleep(100);
                continue;
            }
            else
            {
                // A real error occurred (cable unplugged, etc.)
                log_error("Write error: %s", strerror(errno));
                return false;
            }
        }
        total_sent += n;
    }
    return true;
}

void uart_Close()
{
    if (_uart_uartFd >= 0)
    {
        close(_uart_uartFd);
        _uart_uartFd = -1;
    }
}
