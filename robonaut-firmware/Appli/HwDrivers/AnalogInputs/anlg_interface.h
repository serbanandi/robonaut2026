#ifndef ANLG_INTERFACE_H
#define ANLG_INTERFACE_H

#include <stdint.h>

typedef struct
{
    uint32_t channel[4];
} anlg_DistancesType;

typedef struct
{
    float motorBatteryVoltage;
    float auxBatteryVoltage;
    float motorShuntVoltage;
} anlg_BatteryStatusType;

uint8_t anlg_Init(void);

void anlg_ReadDistances(anlg_DistancesType* distances);

void anlg_ReadBatteryStatus(anlg_BatteryStatusType* status);

#endif // ANLG_INTERFACE_H
