#include "var.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

inline uint32_t var_GetTypeSize(var_VarTypes_t type)
{
    switch (type)
    {
        case VAR_UINT8:
        case VAR_INT8: return 1;
        case VAR_UINT16:
        case VAR_INT16: return 2;
        case VAR_UINT32:
        case VAR_INT32:
        case VAR_FLOAT: return 4;
        default: return 0;
    }
}

inline var_VarValue_t var_GetValue(const char* valueStr, var_VarTypes_t type)
{
    var_VarValue_t value;
    switch (type)
    {
        case VAR_UINT8: value.u8 = (uint8_t) atoi(valueStr); break;
        case VAR_INT8: value.i8 = (int8_t) atoi(valueStr); break;
        case VAR_UINT16: value.u16 = (uint16_t) atoi(valueStr); break;
        case VAR_INT16: value.i16 = (int16_t) atoi(valueStr); break;
        case VAR_UINT32: value.u32 = (uint32_t) atoi(valueStr); break;
        case VAR_INT32: value.i32 = (int32_t) atoi(valueStr); break;
        case VAR_FLOAT: value.f = strtof(valueStr, NULL); break;
        default: memset(&value, 0, sizeof(var_VarValue_t)); break;
    }
    return value;
}

inline void var_GetString(var_VarValue_t value, var_VarTypes_t type, char* outStr, uint32_t outStrSize)
{
    switch (type)
    {
        case VAR_UINT8: snprintf(outStr, outStrSize, "%u", value.u8); break;
        case VAR_INT8: snprintf(outStr, outStrSize, "%d", value.i8); break;
        case VAR_UINT16: snprintf(outStr, outStrSize, "%u", value.u16); break;
        case VAR_INT16: snprintf(outStr, outStrSize, "%d", value.i16); break;
        case VAR_UINT32: snprintf(outStr, outStrSize, "%u", value.u32); break;
        case VAR_INT32: snprintf(outStr, outStrSize, "%d", value.i32); break;
        case VAR_FLOAT: snprintf(outStr, outStrSize, "%.6f", value.f); break;
        default: outStr[0] = '\0'; break;
    }
}