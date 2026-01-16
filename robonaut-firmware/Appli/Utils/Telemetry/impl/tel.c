
#include "tel.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../tel_interface.h"
#include "Uart/uart_interface.h"
#include "usart.h"

// --- Static Variables ---
static uart_UartType tel_uart __NON_CACHEABLE;
static tel_TelemetryVarType registry[TEL_MAX_VARS];
static uint8_t registryCount = 0;

static tel_RxStateType rxState = TEL_RX_WAIT_SOF;
static uint8_t rxBuffer[TEL_RX_BUFFER_SIZE]; // Payload buffer
static uint16_t rxByteCnt = 0;
static uint8_t rxCalcChecksum = 0;
static uint8_t rxEscaped = 0; // Flag for next char being escaped

static uint8_t txTmpBuffer[TEL_TX_BUFFER_SIZE]; // Temporary buffer for transmission
static uint8_t logTmpBuffer[200];               // Temporary buffer for log messages
static uint8_t inputTextBuffer[256] = { 0 };    // Buffer for received text input
static size_t txTmpIndex = 0;
static uint8_t txCalcChecksum = 0;

// --- Helper Functions ---

/**
 * @brief Get the size in bytes of a telemetry variable type.
 * @param type Variable type.
 * @return Size in bytes.
 */
static uint8_t tel_GetTypeSize(tel_VarTypeType type)
{
    switch (type)
    {
        case TEL_UINT8:
        case TEL_INT8: return 1;
        case TEL_UINT16:
        case TEL_INT16: return 2;
        case TEL_UINT32:
        case TEL_INT32:
        case TEL_FLOAT: return 4;
        default: return 0;
    }
}

/**
 * @brief Add a byte to the transmission with escaping and checksum update.
 * @param byte Byte to add.
 * @return 1 on success, 0 on failure.
 */
static uint8_t tel_AddByteEscaped(uint8_t byte)
{
    uint8_t num = 1;
    if (byte == TEL_PROTOCOL_SOF || byte == TEL_PROTOCOL_EOF || byte == TEL_PROTOCOL_ESC)
    {
        num = 2;
    }
    if (txTmpIndex + num > sizeof(txTmpBuffer))
    {
        return 0; // Not enough space
    }

    txCalcChecksum += byte;

    if (num == 1)
    {
        txTmpBuffer[txTmpIndex++] = byte;
    }
    else
    {
        txTmpBuffer[txTmpIndex++] = TEL_PROTOCOL_ESC;
        switch (byte)
        {
            case TEL_PROTOCOL_SOF: txTmpBuffer[txTmpIndex++] = 0x01; break;
            case TEL_PROTOCOL_ESC: txTmpBuffer[txTmpIndex++] = 0x02; break;
            case TEL_PROTOCOL_EOF: txTmpBuffer[txTmpIndex++] = 0x03; break;
        }
    }
    return 1;
}

/**
 * @brief Start a new frame for transmission.
 * @param type Frame type byte.
 * @return 1 on success, 0 on failure.
 */
static uint8_t tel_StartFrame(uint8_t type)
{
    uint8_t sof = TEL_PROTOCOL_SOF;
    txCalcChecksum = 0;
    txTmpIndex = 0;
    txTmpBuffer[txTmpIndex++] = sof;
    return tel_AddByteEscaped(type);
}

/**
 * @brief End the current frame for transmission and send it over UART.
 * @return 1 on success, 0 on failure.
 */
static uint8_t tel_EndFrame()
{
    if (!tel_AddByteEscaped(txCalcChecksum))
    {
        return 0; // Failed to add checksum
    }
    if (txTmpIndex + 1 > sizeof(txTmpBuffer))
    {
        return 0; // Not enough space for EOF
    }
    txTmpBuffer[txTmpIndex++] = TEL_PROTOCOL_EOF;
    return uart_Transmit(&tel_uart, txTmpBuffer, txTmpIndex);
}

/**
 * @brief Process a received packet based on its type.
 */
static void tel_ProcessPacket(void)
{
    uint32_t currentIndex = 0;
    uint8_t rxFrameType = rxBuffer[currentIndex++];
    if ((rxFrameType & TEL_CAT_MASK) == TEL_CAT_REQUEST)
    {
        if (currentIndex >= rxByteCnt)
            return; // Malformed
        uint8_t rxSeqNum = rxBuffer[currentIndex++];
        if ((rxFrameType & TEL_FID_MASK) == TEL_FID_REQ_LIST)
        {
            // LIST REQUEST
            uint8_t type = TEL_CAT_REPLY | TEL_FID_REP_LIST;

            tel_StartFrame(type);
            tel_AddByteEscaped(rxSeqNum);      // Reply with same seq
            tel_AddByteEscaped(registryCount); // N variables

            for (int i = 0; i < registryCount; i++)
            {
                tel_AddByteEscaped(registry[i].id);
                tel_AddByteEscaped((uint8_t) registry[i].type);
                tel_AddByteEscaped(registry[i].accessMode);

                uint8_t len = strlen(registry[i].name);
                tel_AddByteEscaped(len);
                for (int j = 0; j < len; j++)
                {
                    tel_AddByteEscaped((uint8_t) registry[i].name[j]);
                }
            }
            tel_EndFrame();
        }
        else if ((rxFrameType & TEL_FID_MASK) == TEL_FID_REQ_WRITE)
        {
            if (currentIndex >= rxByteCnt)
                return; // Malformed

            uint8_t varId = rxBuffer[currentIndex++];

            if (varId < registryCount && registry[varId].accessMode == TEL_ACCESS_READ_WRITE)
            {
                size_t size = tel_GetTypeSize(registry[varId].type);
                if (rxByteCnt - currentIndex >= size)
                {
                    // Perform Write
                    memcpy(registry[varId].dataPtr, &rxBuffer[currentIndex], size);
                    uint8_t type = TEL_CAT_REPLY | TEL_FID_REP_WRITE;
                    tel_StartFrame(type);
                    tel_AddByteEscaped(rxSeqNum);
                    tel_EndFrame();
                }
            }
        }
    }
    else if ((rxFrameType & TEL_CAT_MASK) == TEL_CAT_STREAM)
    {
        if ((rxFrameType & TEL_FID_MASK) == TEL_FID_TEXT_INPUT)
        {
            // Copy received text into inputTextBuffer
            uint32_t len = (rxByteCnt - currentIndex) < sizeof(inputTextBuffer) - 1 ? (rxByteCnt - currentIndex)
                                                                                    : sizeof(inputTextBuffer) - 1;
            memcpy(inputTextBuffer, &rxBuffer[currentIndex], len);
            inputTextBuffer[len] = '\0'; // Null-terminate
        }
    }
}

/**
 * @brief Parse a received byte from UART.
 * @param byte Received byte.
 */
static void tel_ParseRxByte(uint8_t byte)
{
    // Handle Unescaped SOF/EOF anytime (resync)
    if (byte == TEL_PROTOCOL_SOF)
    {
        rxState = TEL_RX_READ;
        rxCalcChecksum = 0;
        rxEscaped = 0;
        rxByteCnt = 0;
        return;
    }

    switch (rxState)
    {
        case TEL_RX_WAIT_SOF:
            // Do nothing, waiting for SOF (handled at top)
            return;
        case TEL_RX_READ:
            if (byte == TEL_PROTOCOL_ESC)
            {
                rxState = TEL_RX_ESCAPED;
                return;
            }
            if (byte == TEL_PROTOCOL_EOF)
            {
                if (rxByteCnt > 0)
                {
                    uint8_t receivedChecksum = rxBuffer[rxByteCnt - 1];
                    uint8_t calculatedSumWithoutLast = ((int) rxCalcChecksum - (int) receivedChecksum + 256) % 256;

                    if (calculatedSumWithoutLast == receivedChecksum)
                    {
                        // Remove checksum from payload size for processing
                        rxByteCnt--;
                        tel_ProcessPacket();
                    }
                }
                rxState = TEL_RX_WAIT_SOF;
                return;
            }
            if (rxByteCnt < TEL_RX_BUFFER_SIZE)
            {
                rxBuffer[rxByteCnt++] = byte;
                rxCalcChecksum += byte;
            }
            else
            {
                // Buffer overflow
                rxState = TEL_RX_WAIT_SOF;
            }
            break;
        case TEL_RX_ESCAPED:
            // Unescape logic
            switch (byte)
            {
                case 0x01: byte = TEL_PROTOCOL_SOF; break;
                case 0x02: byte = TEL_PROTOCOL_ESC; break;
                case 0x03: byte = TEL_PROTOCOL_EOF; break;
                default:
                    // Invalid escape sequence
                    rxState = TEL_RX_WAIT_SOF;
                    return;
            }
            rxBuffer[rxByteCnt++] = byte;
            rxCalcChecksum += byte;
            rxState = TEL_RX_READ;
            break;
        default: break;
    }
}

// --- Implementation ---

uint8_t tel_Init(void)
{
    registryCount = 0;
    rxState = TEL_RX_WAIT_SOF;
    return uart_Init(&tel_uart, &huart3, USART3_IRQn, TEL_PROTOCOL_EOF);
}

uint8_t tel_RegisterR(void* data, tel_VarTypeType type, const char* name, uint32_t updateRateMs)
{
    if (registryCount >= TEL_MAX_VARS)
        return 0;

    registry[registryCount].dataPtr = data;
    registry[registryCount].type = type;
    registry[registryCount].id = registryCount; // ID is index
    registry[registryCount].accessMode = TEL_ACCESS_READ_ONLY;
    registry[registryCount].updatePeriod = updateRateMs;
    registry[registryCount].lastSent = 0;

    strncpy(registry[registryCount].name, name, TEL_MAX_NAME_LEN - 1);
    registry[registryCount].name[TEL_MAX_NAME_LEN - 1] = '\0';

    registryCount++;
    return 1;
}

uint8_t tel_RegisterRW(void* data, tel_VarTypeType type, const char* name, uint32_t updateRateMs)
{
    if (tel_RegisterR(data, type, name, updateRateMs))
    {
        registry[registryCount - 1].accessMode = TEL_ACCESS_READ_WRITE;
        return 1;
    }
    return 0;
}

uint32_t tel_GetTextInput(char* buffer, uint32_t maxLen)
{
    uint32_t len = strnlen((char*) inputTextBuffer, sizeof(inputTextBuffer));
    if (len == 0)
    {
        return 0; // No new data
    }

    uint32_t copyLen = len < maxLen - 1 ? len : maxLen - 1;
    memcpy(buffer, inputTextBuffer, copyLen);
    buffer[copyLen] = '\0'; // Null-terminate

    // Clear the input buffer
    inputTextBuffer[0] = '\0';

    return copyLen;
}

uint8_t tel_Log(tel_LogLevelType level, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf((char*) logTmpBuffer, sizeof(logTmpBuffer), fmt, args);
    va_end(args);

    uint8_t type = TEL_CAT_STREAM | TEL_FID_LOG_MSG;

    uint8_t ok = tel_StartFrame(type);

    // Payload: Timestamp (4 bytes)
    uint32_t now = HAL_GetTick();
    uint8_t* timePtr = (uint8_t*) &now;
    for (int i = 0; i < 4; i++)
        ok = ok && tel_AddByteEscaped(timePtr[i]);

    // Payload: Log Level (1 byte)
    ok = ok && tel_AddByteEscaped((uint8_t) level);

    // Payload: String
    for (int i = 0; logTmpBuffer[i] != 0; i++)
    {
        ok = ok && tel_AddByteEscaped((uint8_t) logTmpBuffer[i]);
    }

    if (ok)
    {
        return tel_EndFrame();
    }
    return 0;
}

void tel_Process(void)
{
    uint32_t now = HAL_GetTick();

    // 1. Handle Outgoing Streams
    for (int i = 0; i < registryCount; i++)
    {
        if (registry[i].updatePeriod > 0)
        {
            if (now - registry[i].lastSent >= registry[i].updatePeriod)
            {
                // Send Stream Frame
                uint8_t type = TEL_CAT_STREAM | TEL_FID_STREAM_VAL;

                tel_StartFrame(type);

                tel_AddByteEscaped(registry[i].id);

                uint8_t* valPtr = (uint8_t*) registry[i].dataPtr;
                size_t size = tel_GetTypeSize(registry[i].type);

                for (size_t k = 0; k < size; k++)
                {
                    tel_AddByteEscaped(valPtr[k]);
                }

                tel_EndFrame();

                registry[i].lastSent = now;
            }
        }
    }

    // 2. Handle Incoming Data
    // Directly accessing buffer to avoid blocking behavior of uart_Receive
    // or logic issues with termination chars in binary data.

    // We fetch one byte at a time from the UART circular buffer
    uint8_t byte;

    // We loop as long as there is data
    while (1)
    {
        // We use the driver's receive but with max size 1.
        // Note: The driver provided relies on finding a termination char to return 1.
        // This is problematic for binary streams.
        // We will manually pull from the circular buffer structure provided in the header.

        uint32_t bytesLeft = (uint16_t) __HAL_DMA_GET_COUNTER(tel_uart.huart->hdmarx);

        if (tel_uart.readPtr == (UART_READ_BUFFER_LENGTH - bytesLeft) || (bytesLeft == 0 && tel_uart.readPtr == 0))
        {
            break; // Buffer empty (pointers match)
        }

        byte = tel_uart.readCircularBuffer[tel_uart.readPtr];
        tel_uart.readPtr++;
        if (tel_uart.readPtr >= UART_READ_BUFFER_LENGTH)
        {
            tel_uart.readPtr = 0;
        }

        tel_ParseRxByte(byte);
    }
}
