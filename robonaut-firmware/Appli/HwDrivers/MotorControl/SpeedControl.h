#ifndef SPEEDCONTROL_H_
#define SPEEDCONTROL_H_

#include <stdbool.h>

typedef struct {
    float kp;
    float ki;
    float kd;
} SC_SpeedPIDConfig_t;

void SC_Init(HAL_TIM_HandleTypeDef* encoderTimer, HAL_TIM_HandleTypeDef* controlTimer);

void SC_SetEnabled(bool enabled);

void SC_SetTargetSpeed(float speed);

void SC_SetPIDConfig(SC_SpeedPIDConfig_t* config);
void SC_GetPIDConfig(SC_SpeedPIDConfig_t* config);

#endif /* SPEEDCONTROL_H_ */