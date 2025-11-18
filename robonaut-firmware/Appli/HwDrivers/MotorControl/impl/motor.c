#include "motor.h"
#include "main.h"
#include <stdint.h>

#define MOT_MAX_PWR       0.3f

static uint32_t motorTimerPeriod = 0;
static uint32_t minPulseWidth = 0;
static uint32_t maxPulseWidth = 0;
static uint32_t middlePulseWidth = 0;
static uint32_t minMaxPulseWidthDiff = 0;

void mot_Init(void) 
{
    motorTimerPeriod = __HAL_TIM_GET_AUTORELOAD(MOT_PWM_TIMER);
    minPulseWidth = (uint32_t)(0.05f * (motorTimerPeriod + 1)); // 5% duty cycle
    maxPulseWidth = (uint32_t)(0.95f * (motorTimerPeriod + 1)); // 95% duty cycle
    middlePulseWidth = (uint32_t)(0.50f * (motorTimerPeriod + 1)); // 50% duty cycle
    minMaxPulseWidthDiff = (maxPulseWidth - minPulseWidth) / 2;

    HAL_GPIO_WritePin(MOT_EN_GPIO_Port, MOT_EN_Pin, GPIO_PIN_RESET);

    HAL_TIM_PWM_Start(MOT_PWM_TIMER, MOT_PWM1_CHANNEL);
    HAL_TIM_PWM_Start(MOT_PWM_TIMER, MOT_PWM2_CHANNEL);

    // Set initial duty cycle to 50% (stop)
    __HAL_TIM_SET_COMPARE(MOT_PWM_TIMER, MOT_PWM1_CHANNEL, middlePulseWidth);
    __HAL_TIM_SET_COMPARE(MOT_PWM_TIMER, MOT_PWM2_CHANNEL, middlePulseWidth);
}

void mot_Enable(bool enable) 
{
    if (enable) 
    {
        HAL_GPIO_WritePin(MOT_EN_GPIO_Port, MOT_EN_Pin, GPIO_PIN_SET);
    } 
    else 
    {
        HAL_GPIO_WritePin(MOT_EN_GPIO_Port, MOT_EN_Pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(MOT_PWM_TIMER, MOT_PWM1_CHANNEL, middlePulseWidth);
        __HAL_TIM_SET_COMPARE(MOT_PWM_TIMER, MOT_PWM2_CHANNEL, middlePulseWidth);
    }
}

/**
 * @brief Set the motor speed.
 * @param speed The speed to set, in the range [-MOT_MAX_PWR, MOT_MAX_PWR].
 */
void mot_SetSpeed(float speed) 
{
    if (speed < -MOT_MAX_PWR) speed = -MOT_MAX_PWR;
    if (speed >  MOT_MAX_PWR) speed =  MOT_MAX_PWR;

    uint32_t pulseWidth1 = middlePulseWidth + (int32_t)(speed * minMaxPulseWidthDiff);
    uint32_t pulseWidth2 = motorTimerPeriod - pulseWidth1;

    __HAL_TIM_SET_COMPARE(MOT_PWM_TIMER, MOT_PWM1_CHANNEL, pulseWidth1);
    __HAL_TIM_SET_COMPARE(MOT_PWM_TIMER, MOT_PWM2_CHANNEL, pulseWidth2);
}
