#include "ctrl.h"
#include "../ctrl_interface.h"

#include <log.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static var_VarRegistryEntry_t _ctrl_varRegistry[CTRL_MAX_VAR_NUM];
static uint32_t _ctrl_varRegistryCount = 0;

static _ctrl_PendingRequest_t _ctrl_pendingRequests[CTRL_MAX_PENDING_REQUESTS];
static uint32_t _ctrl_pendingRequestCount = 0;
static uint8_t _ctrl_nextSequenceNumber = 0;
static uint32_t _ctrl_nextRequestIndex = 0;

static uint64_t _ctrl_lastVarListRequestTime = 0;

void ctrl_Init()
{
    _ctrl_varRegistryCount = 0;
    _ctrl_pendingRequestCount = 0;
    _ctrl_nextSequenceNumber = 0;
    _ctrl_nextRequestIndex = 0;
    _ctrl_lastVarListRequestTime = 0;
}

void ctrl_UartReceiveCallback(const uint8_t* data, int32_t length)
{
    if (length < 1)
        return;
    switch (data[0] & CTRL_PACKET_CAT_MASK)
    {
        case CTRL_PACKET_CAT_REPLY: _ctrl_HandleUartReply(data[0] & CTRL_PACKET_TYPE_MASK, &data[1], length - 1); break;
        case CTRL_PACKET_CAT_STREAM:
            _ctrl_HandleUartStream(data[0] & CTRL_PACKET_TYPE_MASK, &data[1], length - 1);
            break;
        default: log_warn("Unknown Packet Category 0x%02X", data[0]); break;
    }
}

bool _ctrl_HandleUartReply(uint8_t type, const uint8_t* data, int32_t length)
{
    if (length < 1)
        return false;
    if (type == CTRL_PACKET_TYPE_VAR_LIST_REPLY)
    {
        // Parse Var List
        uint32_t varCount = data[1];
        _ctrl_varRegistryCount = 0;
        uint32_t offset = 2;
        for (uint32_t i = 0; i < varCount; i++)
        {
            _ctrl_varRegistry[i].id = data[offset++];
            _ctrl_varRegistry[i].type = (var_VarTypes_t) data[offset++];
            _ctrl_varRegistry[i].writable = data[offset++] != 0;
            uint8_t nameLen = data[offset++];
            if (nameLen >= VAR_MAX_VARNAME_LENGTH)
                nameLen = VAR_MAX_VARNAME_LENGTH - 1;
            memcpy(_ctrl_varRegistry[i].name, &data[offset], nameLen);
            _ctrl_varRegistry[i].name[nameLen] = 0;
            offset += nameLen;
            _ctrl_varRegistryCount++;
        }
        uint8_t seqNum = data[0];
        log_debug("Received Var List Reply with %d variables and seq %d", _ctrl_varRegistryCount, seqNum);
        _ctrl_CompletePendingRequest(seqNum);
        return true;
    }
    else if (type == CTRL_PACKET_TYPE_VAR_WRITE_REPLY)
    {
        if (length < 1)
            return false;
        uint8_t seqNum = data[0];
        log_debug("Received Var Write Reply for sequence number %d", seqNum);
        _ctrl_CompletePendingRequest(seqNum);
        return true;
    }
    log_warn("Unknown Reply Type 0x%02X", type);
    return false;
}

bool _ctrl_HandleUartStream(uint8_t type, const uint8_t* data, int32_t length)
{
    if (type == CTRL_PACKET_TYPE_LOG)
    {
        if (length < 5)
            return false;
        uint32_t timestampMs;
        memcpy(&timestampMs, &data[0], 4);
        uint8_t level = data[4];
        if (level > 3)
            level = 1;
        char* msg = (char*) malloc(length - 4);
        if (!msg)
            return false;
        memcpy(msg, &data[5], length - 5);
        msg[length - 5] = 0;
        log_debug("Received Log Stream: [%u ms] Level %d: %s", timestampMs, level, msg);
        zmqs_SendLogMessage((zmqs_LogLevel_t) level, timestampMs, msg);
        free(msg);
        return true;
    }
    else if (type == CTRL_PACKET_TYPE_VAR_STREAM)
    {
        if (length < 1)
            return false;
        uint8_t varId = data[0];
        for (uint32_t i = 0; i < _ctrl_varRegistryCount; i++)
        {
            if (_ctrl_varRegistry[i].id == varId)
            {
                var_VarValue_t newValue;
                memcpy(&newValue, &data[1], var_GetTypeSize(_ctrl_varRegistry[i].type));
                _ctrl_varRegistry[i].value = newValue;
                log_debug("Received Var Stream Update for '%s' (ID %d)", _ctrl_varRegistry[i].name, varId);
                zmqs_SendVarValueUpdate(_ctrl_varRegistry[i]);
                return true;
            }
        }
        log_debug("Unknown Var Stream ID %d", data[0]);
    }
    return false;
}

bool ctrl_VarValueUpdateHandler(const char* name, const char* value)
{
    for (uint32_t i = 0; i < _ctrl_varRegistryCount; i++)
    {
        if (strcmp(_ctrl_varRegistry[i].name, name) == 0)
        {
            if (!_ctrl_varRegistry[i].writable)
            {
                log_debug("Var Value Update Handler: Variable '%s' is read-only", name);
                return false;
            }
            var_VarValue_t newValue = var_GetValue(value, _ctrl_varRegistry[i].type);
            // Prepare UART Packet
            uint8_t packet[1 + 1 + 1 + 4];
            uint8_t packetSize = 1 + 1 + 1 + var_GetTypeSize(_ctrl_varRegistry[i].type);
            packet[0] = CTRL_PACKET_CAT_REQUEST | CTRL_PACKET_TYPE_VAR_WRITE;
            packet[1] = _ctrl_nextSequenceNumber++;
            packet[2] = _ctrl_varRegistry[i].id;
            memcpy(&packet[3], &newValue, var_GetTypeSize(_ctrl_varRegistry[i].type));
            // Send via UART
            if (!uart_Send(packet, packetSize))
            {
                log_error("Failed to Send Var Write Packet for '%s'", name);
                return false;
            }
            log_debug("Sent Var Write Packet for '%s'", name);
            _ctrl_PendingRequestData_t requestData = { .varValueUpdate = { .id = _ctrl_varRegistry[i].id,
                                                                           .value = newValue } };
            _ctrl_RegisterPendingRequest(CTRL_PACKET_TYPE_VAR_WRITE, packet[1], requestData);
            return true;
        }
    }
    log_debug("Var Value Update Handler: Variable '%s' not found", name);
    return false;
}

bool ctrl_SendTextInputHandler(const char* inputStr)
{
    // Prepare UART Packet
    uint32_t inputLen = strlen(inputStr);
    uint8_t* packet = (uint8_t*) malloc(1 + inputLen);
    if (!packet)
        return false;
    packet[0] = CTRL_PACKET_CAT_STREAM | CTRL_PACKET_TYPE_TEXT_INPUT;
    memcpy(&packet[1], inputStr, inputLen);
    // Send via UART
    bool result = uart_Send(packet, 1 + inputLen);
    free(packet);
    if (!result)
    {
        log_error("Failed to Send Text Input Packet");
        return false;
    }
    log_debug("Sent Text Input Packet: %s", inputStr);
    return true;
}

uint64_t _ctrl_GetCurrentTimestampMs()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t) (ts.tv_sec) * 1000 + (ts.tv_nsec / 1000000);
}

void _ctrl_RegisterPendingRequest(uint8_t requestType, uint8_t sequenceNumber, _ctrl_PendingRequestData_t data)
{
    uint8_t index = 0;
    if (_ctrl_pendingRequestCount < CTRL_MAX_PENDING_REQUESTS)
    {
        index = _ctrl_pendingRequestCount++;
    }
    else
    {
        index = _ctrl_nextRequestIndex;
        _ctrl_nextRequestIndex = (_ctrl_nextRequestIndex + 1) % CTRL_MAX_PENDING_REQUESTS;
    }
    _ctrl_pendingRequests[index].requestTimestamp = _ctrl_GetCurrentTimestampMs();
    _ctrl_pendingRequests[index].requestType = requestType;
    _ctrl_pendingRequests[index].sequenceNumber = sequenceNumber;
    _ctrl_pendingRequests[index].awaitingReply = true;
    _ctrl_pendingRequests[index].data = data;
}

void _ctrl_CompletePendingRequest(uint8_t sequenceNumber)
{
    for (uint32_t i = 0; i < _ctrl_pendingRequestCount; i++)
    {
        if (_ctrl_pendingRequests[i].sequenceNumber == sequenceNumber && _ctrl_pendingRequests[i].awaitingReply)
        {
            _ctrl_pendingRequests[i].awaitingReply = false;
            if (_ctrl_pendingRequests[i].requestType == CTRL_PACKET_TYPE_VAR_WRITE)
            {
                // Update Local Registry
                uint8_t varId = _ctrl_pendingRequests[i].data.varValueUpdate.id;
                for (uint32_t j = 0; j < _ctrl_varRegistryCount; j++)
                {
                    if (_ctrl_varRegistry[j].id == varId)
                    {
                        _ctrl_varRegistry[j].value = _ctrl_pendingRequests[i].data.varValueUpdate.value;
                        log_info("Var ID %d Updated Successfully", varId);
                        zmqs_SendVarValueUpdate(_ctrl_varRegistry[j]);
                        break;
                    }
                }
            }
            break;
        }
    }
}

void _ctrl_VarListRequestHandler()
{
    // Prepare UART Packet
    uint8_t packet[1 + 1];
    packet[0] = CTRL_PACKET_CAT_REQUEST | CTRL_PACKET_TYPE_VAR_LIST;
    packet[1] = _ctrl_nextSequenceNumber++;
    // Send via UART
    if (!uart_Send(packet, 2))
    {
        log_error("Failed to Send Var List Request Packet");
        return;
    }
    _ctrl_PendingRequestData_t requestData = { 0 };
    log_debug("Sent Var List Request Packet");
    _ctrl_RegisterPendingRequest(CTRL_PACKET_TYPE_VAR_LIST, packet[1], requestData);
}

void ctrl_PeriodicTask()
{
    uint64_t currentTime = _ctrl_GetCurrentTimestampMs();
    for (uint32_t i = 0; i < _ctrl_pendingRequestCount; i++)
    {
        if (_ctrl_pendingRequests[i].awaitingReply && (currentTime - _ctrl_pendingRequests[i].requestTimestamp) > 100)
        {
            log_warn("CTRL: Pending Request Type 0x%02X Seq 0x%02X Timed Out", _ctrl_pendingRequests[i].requestType,
                     _ctrl_pendingRequests[i].sequenceNumber);
            _ctrl_pendingRequests[i].awaitingReply = false;
        }
    }
    if ((currentTime - _ctrl_lastVarListRequestTime) > CTRL_VAR_LIST_POLL_INTERVAL_MS)
    {
        _ctrl_VarListRequestHandler();
        _ctrl_lastVarListRequestTime = currentTime;
    }
}

void ctrl_GetVarRegistry(var_VarRegistryEntry_t** registry, uint32_t* count)
{
    *registry = _ctrl_varRegistry;
    *count = _ctrl_varRegistryCount;
}
