#ifndef VAR_TYPES_H
#define VAR_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#define VAR_MAX_VARNAME_LENGTH 50

typedef enum
{
    VAR_UINT8 = 0,
    VAR_INT8,
    VAR_UINT16,
    VAR_INT16,
    VAR_UINT32,
    VAR_INT32,
    VAR_FLOAT
} var_VarTypes_t;

typedef union
{
    uint8_t u8;
    int8_t i8;
    uint16_t u16;
    int16_t i16;
    uint32_t u32;
    int32_t i32;
    float f;
} var_VarValue_t;

typedef struct
{
    var_VarTypes_t type;
    char name[VAR_MAX_VARNAME_LENGTH];
    uint8_t id;
    bool writable;
    var_VarValue_t value;
} var_VarRegistryEntry_t;

uint32_t var_GetTypeSize(var_VarTypes_t type);

var_VarValue_t var_GetValue(const char* valueStr, var_VarTypes_t type);

void var_GetString(var_VarValue_t value, var_VarTypes_t type, char* outStr, uint32_t outStrSize);

#endif // VAR_TYPES_H
