#include "drv_motor.h"
#include <stdint.h>
#include "main.h"

static uint32_t motorTimerPeriod = 0;
static uint32_t minPulseWidth = 0;
static uint32_t maxPulseWidth = 0;
static uint32_t middlePulseWidth = 0;
static uint32_t minMaxPulseWidthDiff = 0;
static float _drv_maxPower = 0.5f; // Maximum power level

void _drv_MotorInit(void)
{
    motorTimerPeriod = __HAL_TIM_GET_AUTORELOAD(DRV_PWM_TIMER);
    minPulseWidth = (uint32_t) (0.05f * (motorTimerPeriod + 1));    // 5% duty cycle
    maxPulseWidth = (uint32_t) (0.95f * (motorTimerPeriod + 1));    // 95% duty cycle
    middlePulseWidth = (uint32_t) (0.50f * (motorTimerPeriod + 1)); // 50% duty cycle
    minMaxPulseWidthDiff = (maxPulseWidth - minPulseWidth) / 2;

    HAL_GPIO_WritePin(MOT_EN_GPIO_Port, MOT_EN_Pin, GPIO_PIN_RESET);

    HAL_TIM_PWM_Start(DRV_PWM_TIMER, DRV_PWM1_CHANNEL);
    HAL_TIM_PWM_Start(DRV_PWM_TIMER, DRV_PWM2_CHANNEL);

    // Set initial duty cycle to 50% (stop)
    __HAL_TIM_SET_COMPARE(DRV_PWM_TIMER, DRV_PWM1_CHANNEL, middlePulseWidth);
    __HAL_TIM_SET_COMPARE(DRV_PWM_TIMER, DRV_PWM2_CHANNEL, middlePulseWidth);
}

void _drv_EnableMotor(bool enable)
{
    if (enable)
    {
        HAL_GPIO_WritePin(MOT_EN_GPIO_Port, MOT_EN_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(MOT_EN_GPIO_Port, MOT_EN_Pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(DRV_PWM_TIMER, DRV_PWM1_CHANNEL, middlePulseWidth);
        __HAL_TIM_SET_COMPARE(DRV_PWM_TIMER, DRV_PWM2_CHANNEL, middlePulseWidth);
    }
}

void _drv_SetPower(float power)
{
    if (power < -_drv_maxPower)
        power = -_drv_maxPower;
    if (power > _drv_maxPower)
        power = _drv_maxPower;

    uint32_t pulseWidth1 = middlePulseWidth + (int32_t) (-power * minMaxPulseWidthDiff);
    uint32_t pulseWidth2 = motorTimerPeriod - pulseWidth1;

    __HAL_TIM_SET_COMPARE(DRV_PWM_TIMER, DRV_PWM1_CHANNEL, pulseWidth1);
    __HAL_TIM_SET_COMPARE(DRV_PWM_TIMER, DRV_PWM2_CHANNEL, pulseWidth2);
}

void drv_SetMaxPower(float maxPower)
{
    if (maxPower < 0.0f)
        maxPower = 0.0f;
    if (maxPower > 1.0f)
        maxPower = 1.0f;
    _drv_maxPower = maxPower;
}
