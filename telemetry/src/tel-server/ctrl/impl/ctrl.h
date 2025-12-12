#ifndef CTRL_H
#define CTRL_H

#include "var.h"

#include <stdbool.h>
#include <stdint.h>

#define CTRL_PACKET_CAT_MASK 0xC0
#define CTRL_PACKET_TYPE_MASK ~CTRL_PACKET_CAT_MASK

#define CTRL_PACKET_CAT_REQUEST 0x00
#define CTRL_PACKET_CAT_REPLY 0x40
#define CTRL_PACKET_CAT_STREAM 0x80

#define CTRL_PACKET_TYPE_VAR_LIST 0x01
#define CTRL_PACKET_TYPE_VAR_LIST_REPLY 0x01
#define CTRL_PACKET_TYPE_VAR_WRITE 0x02
#define CTRL_PACKET_TYPE_VAR_WRITE_REPLY 0x02
#define CTRL_PACKET_TYPE_LOG 0x01
#define CTRL_PACKET_TYPE_TEXT_INPUT 0x02
#define CTRL_PACKET_TYPE_VAR_STREAM 0x03

#define CTRL_MAX_PENDING_REQUESTS 100
#define CTRL_MAX_VAR_NUM 100

#define CTRL_VAR_LIST_POLL_INTERVAL_MS 5000

typedef struct
{
    uint8_t id;
    var_VarValue_t value;
} _ctrl_VarValueUpdate_t;

typedef union
{
    _ctrl_VarValueUpdate_t varValueUpdate;
} _ctrl_PendingRequestData_t;

typedef struct
{
    uint64_t requestTimestamp;
    uint8_t requestType;
    uint8_t sequenceNumber;
    bool awaitingReply;
    _ctrl_PendingRequestData_t data;
} _ctrl_PendingRequest_t;

bool _ctrl_HandleUartReply(uint8_t type, const uint8_t* data, int32_t length);
bool _ctrl_HandleUartStream(uint8_t type, const uint8_t* data, int32_t length);

void _ctrl_RegisterPendingRequest(uint8_t requestType, uint8_t sequenceNumber, _ctrl_PendingRequestData_t data);
void _ctrl_CompletePendingRequest(uint8_t sequenceNumber);

void _ctrl_VarListRequestHandler();

uint64_t _ctrl_GetCurrentTimestampMs();

#endif // CTRL_H
