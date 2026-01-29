#include "anlg.h"
#include "../anlg_interface.h"
#include "IntHandler/int_interface.h"

#include "adc.h"

static uint32_t _anlg_rawAdc1Channels[4] __NON_CACHEABLE = { 0 };
static uint32_t _anlg_rawAdc2Channels[3] __NON_CACHEABLE = { 0 };

uint8_t anlg_Init(void)
{
    HAL_StatusTypeDef c1 = HAL_ADCEx_Calibration_Start(ANLG_DISTANCE_ADC, ADC_SINGLE_ENDED);
    HAL_StatusTypeDef c2 = HAL_ADCEx_Calibration_Start(ANLG_BATTERY_ADC, ADC_SINGLE_ENDED);

    HAL_StatusTypeDef c3 = HAL_ADC_Start_DMA(ANLG_DISTANCE_ADC, _anlg_rawAdc1Channels, 4);
    HAL_StatusTypeDef c4 = HAL_ADC_Start_DMA(ANLG_BATTERY_ADC, _anlg_rawAdc2Channels, 3);
    return (c1 == HAL_OK && c2 == HAL_OK && c3 == HAL_OK && c4 == HAL_OK) ? 1 : 0;
}

void anlg_ReadDistances(anlg_DistancesType* distances) // TODO
{
    distances->channel[0] = 69;
    distances->channel[1] = 69;
    distances->channel[2] = 69;
    distances->channel[3] = 69;
}

void anlg_ReadBatteryStatus(anlg_BatteryStatusType* status)
{
    status->auxBatteryVoltage = (float) _anlg_rawAdc2Channels[0] / ANLG_AUX_BATTERY_COUNTS_PER_VOLT;
    status->motorBatteryVoltage = (float) _anlg_rawAdc2Channels[1] / ANLG_MOTOR_BATTERY_COUNTS_PER_VOLT;
    status->motorShuntVoltage = (float) _anlg_rawAdc2Channels[2] / ANLG_MOTOR_SHUNT_COUNTS_PER_AMP;
}
