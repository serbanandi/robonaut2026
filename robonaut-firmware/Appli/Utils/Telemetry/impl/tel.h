#ifndef TEL_H
#define TEL_H

#include <stdint.h>
#include "../tel_interface.h"

// Configuration
#define TEL_MAX_VARS                    50      // Maximum number of registered variables
#define TEL_MAX_NAME_LEN                30      // Maximum length of a variable name
#define TEL_RX_BUFFER_SIZE              800     // Internal buffer for parsing frames
#define TEL_TX_BUFFER_SIZE              ((TEL_MAX_NAME_LEN + 10) * TEL_MAX_VARS)    // Internal buffer for building frames

// --- Protocol Constants ---
#define TEL_PROTOCOL_SOF                42  // 10'42
#define TEL_PROTOCOL_EOF                69  // 10'69
#define TEL_PROTOCOL_ESC                123 // 10'123

#define TEL_CAT_MASK                    0xC0 // Top 2 bits
#define TEL_FID_MASK                    0x3F // Bottom 6 bits

// Frame Categories (Top 2 bits of Type Byte)
#define TEL_CAT_REQUEST                 0x00
#define TEL_CAT_REPLY                   0x40 // Binary 01xxxxxx
#define TEL_CAT_STREAM                  0x80 // Binary 10xxxxxx

// Frame IDs (Bottom 6 bits)
#define TEL_FID_LOG_MSG                 0x01
#define TEL_FID_TEXT_INPUT              0x02
#define TEL_FID_STREAM_VAL              0x03
#define TEL_FID_REQ_LIST                0x01
#define TEL_FID_REQ_WRITE               0x02
#define TEL_FID_REP_LIST                0x01
#define TEL_FID_REP_WRITE               0x02

// Access Types
#define TEL_ACCESS_READ_ONLY        0
#define TEL_ACCESS_READ_WRITE       1

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

// RX State Machine
typedef enum {
    TEL_RX_WAIT_SOF,
    TEL_RX_READ,
    TEL_RX_ESCAPED
} tel_RxStateType;

#endif // TEL_H