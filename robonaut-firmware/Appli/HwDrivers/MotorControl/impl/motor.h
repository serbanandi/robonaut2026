#ifndef MOTOR_H
#define MOTOR_H

#include "../mot_interface.h"
#include "stm32n6xx_hal.h"
#include "tim.h"

#define MOT_PWM_TIMER &htim1
#define MOT_PWM1_CHANNEL TIM_CHANNEL_2
#define MOT_PWM2_CHANNEL TIM_CHANNEL_1

#endif // MOTOR_H