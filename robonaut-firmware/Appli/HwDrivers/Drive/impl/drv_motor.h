#ifndef DRV_MOTOR_H
#define DRV_MOTOR_H

#include "../drv_interface.h"
#include "stm32n6xx_hal.h"
#include "tim.h"

#define DRV_PWM_TIMER &htim1
#define DRV_PWM1_CHANNEL TIM_CHANNEL_2
#define DRV_PWM2_CHANNEL TIM_CHANNEL_1

void _drv_MotorInit(void);

void _drv_EnableMotor(bool enable);

void _drv_SetPower(float power);

void _drv_GetMaxPower(float *maxPower);

#endif // DRV_MOTOR_H