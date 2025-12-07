
#include "../tel_interface.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include "usart.h"


// Configuration
#define TEL_MAX_VARS            30      // Maximum number of registered variables
#define TEL_MAX_NAME_LEN        10      // Maximum length of a variable name
#define TEL_RX_BUFFER_SIZE      800     // Internal buffer for parsing frames
#define TEL_TX_BUFFER_SIZE      800     // Internal buffer for building frames

// --- Protocol Constants ---
#define PROTOCOL_SOF            42  // 10'42
#define PROTOCOL_EOF            69  // 10'69
#define PROTOCOL_ESC            123 // 10'123

// Frame Categories (Top 2 bits of Type Byte)
#define CAT_LOG                 0x00
#define CAT_REQUEST             0x40 // Binary 01xxxxxx
#define CAT_REPLY               0x80 // Binary 10xxxxxx
#define CAT_STREAM              0xC0 // Binary 11xxxxxx

// Frame IDs (Bottom 6 bits)
#define FID_LOG_MSG             0x01
#define FID_REQ_LIST            0x01
#define FID_REQ_WRITE           0x02
#define FID_REP_LIST            0x01
#define FID_REP_WRITE           0x02
#define FID_STREAM_VAL          0x01

// Access Types
#define ACCESS_READ_ONLY        0
#define ACCESS_READ_WRITE       1

// --- Internal Structures ---
typedef struct {
    void* dataPtr;
    char        name[TEL_MAX_NAME_LEN];
    uint8_t     id;
    tel_VarTypeType type;
    uint8_t     accessMode; // 0=RO, 1=RW
    uint32_t    updatePeriod;
    uint32_t    lastSent;
} tel_TelemetryVarType;

// --- Static Variables ---
static uart_UartType tel_uart;
static tel_TelemetryVarType registry[TEL_MAX_VARS];
static uint8_t registryCount = 0;

// RX State Machine
typedef enum {
    RX_WAIT_SOF,
    RX_READ_TYPE,
    RX_READ_SEQ,
    RX_READ_PAYLOAD,
    RX_WAIT_EOF
} tel_RxStateType;

static tel_RxStateType rxState = RX_WAIT_SOF;
static uint8_t rxBuffer[TEL_RX_BUFFER_SIZE]; // Payload buffer
static uint16_t rxIndex = 0;
static uint8_t rxFrameType = 0;
static uint8_t rxSeqNum = 0;
static uint8_t rxCalcChecksum = 0;
static uint8_t rxEscaped = 0; // Flag for next char being escaped

static uint8_t txTmpBuffer[256]; // Temporary buffer for transmission
static size_t txTmpIndex = 0;
static uint8_t txCalcChecksum = 0;

// --- Helper Functions ---

static uint8_t tel_GetTypeSize(tel_VarTypeType type) 
{
    switch(type) 
    {
        case TEL_UINT8: case TEL_INT8: return 1;
        case TEL_UINT16: case TEL_INT16: return 2;
        case TEL_UINT32: case TEL_INT32: case TEL_FLOAT: return 4;
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
    if (byte == PROTOCOL_SOF || byte == PROTOCOL_EOF || byte == PROTOCOL_ESC) 
    {
        num = 2;
    }
    if (txTmpIndex + num > sizeof(txTmpBuffer)) 
    {
        return 0; // Not enough space
    }
    
    if (num == 1)
    {
        txTmpBuffer[txTmpIndex++] = byte;
        txCalcChecksum += byte;
    } 
    else 
    {
        txTmpBuffer[txTmpIndex++] = PROTOCOL_ESC;
        txCalcChecksum += PROTOCOL_ESC;
        switch(byte)
        {
            case PROTOCOL_SOF:
                txTmpBuffer[txTmpIndex++] = 0x01;
                break;
            case PROTOCOL_EOF:
                txTmpBuffer[txTmpIndex++] = 0x02;
                txCalcChecksum += PROTOCOL_ESC;
                break;
            case PROTOCOL_ESC:
                txTmpBuffer[txTmpIndex++] = 0x03;
                txCalcChecksum += PROTOCOL_ESC;
                break;
        }
        txCalcChecksum += txTmpBuffer[txTmpIndex - 1];
    }
}

/**
 * @brief Start a new frame for transmission.
 * @param type Frame type byte.
 * @return 1 on success, 0 on failure.
 */
static uint8_t tel_StartFrame(uint8_t type) 
{
    uint8_t sof = PROTOCOL_SOF;
    txCalcChecksum = 0;
    txTmpIndex = 0;
    txTmpBuffer[txTmpIndex++] = sof;
    return tel_AddByteEscaped(type);
}

/**
 * @brief End the current frame for transmission.
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
    txTmpBuffer[txTmpIndex++] = PROTOCOL_EOF;
    return uart_Transmit(&tel_uart, txTmpBuffer, txTmpIndex);
}

// --- Implementation ---

void tel_init(void) 
{
    uart_Init(&tel_uart, &huart3, USART3_IRQn, '\n');
    registryCount = 0;
    rxState = RX_WAIT_SOF;
}

uint8_t tel_registerR(void* data, tel_VarTypeType type, const char* name, uint32_t updateRateMs) 
{
    if (registryCount >= TEL_MAX_VARS) 
        return 0;
    
    registry[registryCount].dataPtr = data;
    registry[registryCount].type = type;
    registry[registryCount].id = registryCount; // ID is index
    registry[registryCount].accessMode = ACCESS_READ_ONLY;
    registry[registryCount].updatePeriod = updateRateMs;
    registry[registryCount].lastSent = 0;
    
    strncpy(registry[registryCount].name, name, TEL_MAX_NAME_LEN - 1);
    registry[registryCount].name[TEL_MAX_NAME_LEN - 1] = '\0';
    
    registryCount++;
    return 1;
}

uint8_t tel_registerRW(void* data, tel_VarTypeType type, const char* name, uint32_t updateRateMs) 
{
    if(tel_registerR(data, type, name, updateRateMs)) 
    {
        registry[registryCount-1].accessMode = ACCESS_READ_WRITE;
        return 1;
    }
    return 0;
}

void tel_log(tel_LogLevelType level, const char* fmt, ...) 
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(txTmpBuffer, sizeof(txTmpBuffer), fmt, args);
    va_end(args);

    uint8_t checksum = 0;
    uint8_t type = CAT_LOG | FID_LOG_MSG;
    
    uint8_t ok = tel_StartFrame(type);
    
    // Payload: Timestamp (4 bytes)
    uint32_t now = HAL_GetTick();
    uint8_t* timePtr = (uint8_t*)&now;
    for(int i=0; i<4; i++) 
        ok = ok && tel_AddByteEscaped(timePtr[i]);

    // Payload: Log Level (1 byte)
    ok = ok && tel_AddByteEscaped((uint8_t)level);
    
    // Payload: String
    for(int i=0; txTmpBuffer[i] != 0; i++) 
    {
        ok = ok && tel_AddByteEscaped((uint8_t)txTmpBuffer[i]);
    }
    
    if (ok) 
    {
        return tel_EndFrame();
    }
    return 0;
}

// Process a successfully received packet
static void tel_ProcessPacket(void) 
{
    // 1. LIST REQUEST
    if (rxFrameType == (CAT_REQUEST | FID_REQ_LIST)) 
    {
        uint8_t checksum = 0;
        uint8_t type = CAT_REPLY | FID_REP_LIST;
        
        tel_StartFrame(type);
        tel_AddByteEscaped(rxSeqNum); // Reply with same seq
        
        tel_AddByteEscaped(registryCount); // N variables
        
        for(int i=0; i<registryCount; i++) 
        {
            tel_AddByteEscaped(registry[i].id);
            tel_AddByteEscaped((uint8_t)registry[i].type);
            tel_AddByteEscaped(registry[i].accessMode);
            
            uint8_t len = strlen(registry[i].name);
            tel_AddByteEscaped(len);
            for(int j=0; j<len; j++) 
            {
                tel_AddByteEscaped((uint8_t)registry[i].name[j]);
            }
        }
        tel_EndFrame();
    }
    // 2. WRITE REQUEST
    else if (rxFrameType == (CAT_REQUEST | FID_REQ_WRITE)) 
    {
        if(rxIndex < 1) return; // Malformed
        
        uint8_t varId = rxBuffer[0];
        
        if(varId < registryCount && registry[varId].accessMode == ACCESS_READ_WRITE) 
        {
            size_t size = tel_GetTypeSize(registry[varId].type);
            if((rxIndex - 1) >= size) 
            {
                // Perform Write
                memcpy(registry[varId].dataPtr, &rxBuffer[1], size);
                
                // Send Reply
                uint8_t checksum = 0;
                uint8_t type = CAT_REPLY | FID_REP_WRITE;
                tel_StartFrame(type, &checksum);
                tel_AddByteEscaped(rxSeqNum, &checksum);
                tel_EndFrame(checksum);
            }
        }
    }
}

// Handle RX byte by byte to manage state machine and escaping
static void tel_ParseRxByte(uint8_t byte) 
{
    // Handle Unescaped SOF/EOF anytime (resync)
    if (byte == PROTOCOL_SOF) 
    {
        rxState = RX_READ_TYPE;
        rxCalcChecksum = 0;
        rxEscaped = 0;
        return;
    }
    
    // Handle Escape char
    if (byte == PROTOCOL_ESC) 
    {
        rxEscaped = 1;
        return;
    }
    
    // Unescape logic
    if (rxEscaped) 
    {
        if (byte == 1) 
            byte = PROTOCOL_SOF;
        else if (byte == 2) 
            byte = PROTOCOL_ESC;
        else if (byte == 3) 
            byte = PROTOCOL_EOF;
        else 
        {
            // Invalid escape sequence
            rxState = RX_WAIT_SOF;
            rxEscaped = 0;
            return;
        }
        rxEscaped = 0;
    }

    // State Machine
    switch (rxState) 
    {
        case RX_WAIT_SOF:
            // Do nothing, waiting for SOF (handled at top)
            break;
            
        case RX_READ_TYPE:
            rxFrameType = byte;
            rxCalcChecksum += byte;
            rxIndex = 0;
            
            // Determine if we expect a Sequence Number
            // Categories 1 (Request) and 2 (Reply) have SeqNum
            if ((rxFrameType & 0xC0) == CAT_REQUEST || (rxFrameType & 0xC0) == CAT_REPLY) 
            {
                rxState = RX_READ_SEQ;
            } 
            else 
            {
                rxState = RX_READ_PAYLOAD;
            }
            break;
            
        case RX_READ_SEQ:
            rxSeqNum = byte;
            rxCalcChecksum += byte;
            rxState = RX_READ_PAYLOAD;
            break;
            
        case RX_READ_PAYLOAD:
            if (byte == PROTOCOL_EOF) 
            {
                if (rxIndex > 0)
                {
                    uint8_t receivedChecksum = rxBuffer[rxIndex-1];
                    uint8_t calculatedSumWithoutLast = rxCalcChecksum - receivedChecksum;
                    
                    if (calculatedSumWithoutLast == receivedChecksum) 
                    {
                        // Remove checksum from payload size for processing
                        rxIndex--; 
                        tel_ProcessPacket();
                    }
                }
                rxState = RX_WAIT_SOF;
                break;
            }
            if (rxIndex < TEL_RX_BUFFER_SIZE) 
            {
                rxBuffer[rxIndex++] = byte;
                rxCalcChecksum += byte;
            }
            else 
            {
                // Buffer overflow
                rxState = RX_WAIT_SOF;
            }
            break;
            
        default:
            rxState = RX_WAIT_SOF;
            break;
    }
}

void tel_process(void) {
    uint32_t now = HAL_GetTick();

    // 1. Handle Outgoing Streams
    for(int i=0; i<registryCount; i++) {
        if(registry[i].updatePeriod > 0) {
            if(now - registry[i].lastSent >= registry[i].updatePeriod) {
                // Send Stream Frame
                uint8_t checksum = 0;
                uint8_t type = CAT_STREAM | FID_STREAM_VAL;
                
                tel_StartFrame(type, &checksum);
                
                tel_AddByteEscaped(registry[i].id, &checksum);
                
                uint8_t* valPtr = (uint8_t*)registry[i].dataPtr;
                size_t size = tel_GetTypeSize(registry[i].type);
                
                for(size_t k=0; k<size; k++) {
                    tel_AddByteEscaped(valPtr[k], &checksum);
                }
                
                tel_EndFrame(checksum);
                
                registry[i].lastSent = now;
            }
        }
    }

    // 2. Handle Incoming Data
    // Directly accessing buffer to avoid blocking behavior of uart_Receive
    // or logic issues with termination chars in binary data.
    
    // We fetch one byte at a time from the UART circular buffer
    size_t receivedLen;
    uint8_t byte;
    
    // We loop as long as there is data
    while(1) {
        // We use the driver's receive but with max size 1. 
        // Note: The driver provided relies on finding a termination char to return 1.
        // This is problematic for binary streams. 
        // We will manually pull from the circular buffer structure provided in the header.
        
        if (tel_uart->readPtr == (UART_READ_BUFFER_LENGTH - tel_uart->huart->RxXferCount)) {
            break; // Buffer empty (pointers match)
        }
        
        byte = tel_uart->readCircularBuffer[tel_uart->readPtr];
        tel_uart->readPtr++;
        if (tel_uart->readPtr >= UART_READ_BUFFER_LENGTH) {
            tel_uart->readPtr = 0;
        }
        
        tel_ParseRxByte(byte);
    }
}