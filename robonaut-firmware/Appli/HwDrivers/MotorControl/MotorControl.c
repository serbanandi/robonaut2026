#include "MotorControl.h"

typedef struct {
    TIM_HandleTypeDef* pwmTimer;
    bool enabled;
    float power; // Power level from -100 to 100
} MotorControl_t;

static MotorControl_t motorControl;

void MC_Init(TIM_HandleTypeDef* pwmTimer) {
    motorControl.pwmTimer = pwmTimer;
    motorControl.enabled = false;
    motorControl.power = 0.0f;

    HAL_TIM_PWM_Start(motorControl.pwmTimer, TIM_CHANNEL_1);
}

void MC_SetPower(int power) {
    if (!motorControl.enabled) {
        return;
    }

    if (power > 100) power = 100;
    if (power < -100) power = -100;

    motorControl.power = (float)power;

    uint32_t pulseWidth = (uint32_t)((motorControl.power + 100) / 200.0f * (__HAL_TIM_GET_AUTORELOAD(motorControl.pwmTimer) + 1));
    __HAL_TIM_SET_COMPARE(motorControl.pwmTimer, TIM_CHANNEL_1, pulseWidth);
}

void MC_Enable(bool enable) {
    motorControl.enabled = enable;
    if (!enable) {
        __HAL_TIM_SET_COMPARE(motorControl.pwmTimer, TIM_CHANNEL_1, 0);
    }
}
