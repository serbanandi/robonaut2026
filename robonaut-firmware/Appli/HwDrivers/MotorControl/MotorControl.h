#ifndef MOTORCONTROL_H_
#define MOTORCONTROL_H_

#include <stdbool.h>
#include "stm32n6xx_hal.h"

void MC_Init(TIM_HandleTypeDef* pwmTimer);

void MC_Enable(bool enable);

void MC_SetPower(int power);

#endif /* MOTORCONTROL_H_ */
