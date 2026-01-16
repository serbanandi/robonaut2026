#ifndef SERVO_H
#define SERVO_H

#include "../servo_interface.h"
#include "stm32n6xx_hal.h"
#include "tim.h"

#define SERVO_TIMER &htim15
#define SERVO_MIN_PULSE_WIDTH 1200 // in microseconds
#define SERVO_MAX_PULSE_WIDTH 2050 // in microseconds
#define SERVO_FRONT_CHANNEL TIM_CHANNEL_1
#define SERVO_BACK_CHANNEL TIM_CHANNEL_2

#endif // SERVO_H
