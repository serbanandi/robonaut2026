#ifndef LS_INTERFACE_H_
#define LS_INTERFACE_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    LS_SENSOR_FRONT = 0,
    LS_SENSOR_REAR = 1
} ls_SensorPositionType;

typedef struct
{
    uint16_t v[32];
} ls_AdcValuesType;

typedef struct
{
    bool v[32];
} ls_LedValuesType;

bool ls_Init(void);

bool ls_Process(bool* newDataAvailable);

void ls_GetADCValues(ls_AdcValuesType* values, const ls_SensorPositionType sensor);

bool ls_SetFbLEDs(const ls_LedValuesType* values, const ls_SensorPositionType sensor);

#endif /* LS_INTERFACE_H_ */
