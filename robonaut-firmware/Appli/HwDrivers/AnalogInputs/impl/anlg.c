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

void anlg_ReadDistances(anlg_DistancesType* distances)
{
    // IR1, IR2 - Sharp GP2Y0A02YK0F (20cm-150cm)
    // Conversion: 45.0/(v-0.2)
    float v1 = (float) _anlg_rawAdc1Channels[0] / 4095.0f * 3.3f;
    float v2 = (float) _anlg_rawAdc1Channels[1] / 4095.0f * 3.3f;
    float dist1 = 45.0f / (v1 - 0.2f);
    float dist2 = 45.0f / (v2 - 0.2f);
    distances->channel[0] = (uint32_t) (dist1 > 150.0) ? 150 : ((dist1 < 15.0) ? 15 : dist1);
    distances->channel[1] = (uint32_t) (dist2 > 150.0) ? 150 : ((dist2 < 15.0) ? 15 : dist2);

    // IR3, IR4 - Sharp GP2Y0A41SK0F (4cm-30cm)
    // Conversion: (10.5/(v-0.15))-0.42
    float v3 = (float) _anlg_rawAdc1Channels[2] / 4095.0f * 3.3f;
    float v4 = (float) _anlg_rawAdc1Channels[3] / 4095.0f * 3.3f;
    float dist3 = (10.5f / (v3 - 0.15f)) - 0.42f;
    float dist4 = (10.5f / (v4 - 0.15f)) - 0.42f;
    distances->channel[2] = (uint32_t) (dist3 > 40.0) ? 40 : ((dist3 < 4.0) ? 4 : dist3);
    distances->channel[3] = (uint32_t) (dist4 > 40.0) ? 40 : ((dist4 < 4.0) ? 4 : dist4);ű
}

void anlg_ReadBatteryStatus(anlg_BatteryStatusType* status)
{
    status->auxBatteryVoltage = (float) _anlg_rawAdc2Channels[0] / ANLG_AUX_BATTERY_COUNTS_PER_VOLT;
    status->motorBatteryVoltage = (float) _anlg_rawAdc2Channels[1] / ANLG_MOTOR_BATTERY_COUNTS_PER_VOLT;
    status->motorShuntVoltage = (float) _anlg_rawAdc2Channels[2] / ANLG_MOTOR_SHUNT_COUNTS_PER_AMP;
}
