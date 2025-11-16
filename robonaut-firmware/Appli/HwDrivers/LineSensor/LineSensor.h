#ifndef LINESENSOR_H_
#define LINESENSOR_H_

#include <stdbool.h>
#include "stm32n6xx_hal.h"

bool LS_Init(SPI_HandleTypeDef* front_spi, SPI_HandleTypeDef* rear_spi);
void LS_Process(void);

typedef struct {
    uint16_t front_adc[32];
    uint16_t rear_adc[32];
} LS_ADC_Values_Type;

typedef struct {
    bool front_led[32];
    bool rear_led[32];
} LS_LED_Values_Type;

void LS_GetADCValues(LS_ADC_Values_Type* adc_values);

bool LS_SetFbLEDs(const LS_LED_Values_Type* led_values);

#endif /* LINESENSOR_H_ */
