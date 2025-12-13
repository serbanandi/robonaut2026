#include "../drv_interface.h"
#include "drv_motor.h"
#include "drv_encoder.h"
#include "drv.h"
#include "IntHandler/int_interface.h"

static drv_ControlParamsType DRV_CTRL_PARAMS;

static volatile float previousError;
static volatile float errorIntegral;

static volatile int32_t lastEncoderDiff;

void _drv_TimerInterruptHandler(void*)
{
    uint32_t currentEncoderCount = drv_GetEncoderCount();
    static uint32_t lastEncoderCount = 0;
    int64_t encoderDiff = ((int64_t)currentEncoderCount - (int64_t)lastEncoderCount);
    // Handle encoder count overflow/underflow
    if (encoderDiff > 0x7FFFFFFF)
        encoderDiff -= 0x100000000;
    else if (encoderDiff < -0x80000000)
        encoderDiff += 0x100000000;
    lastEncoderDiff = (int32_t)encoderDiff;
    lastEncoderCount = currentEncoderCount;
}

void drv_Init(const drv_ControlParamsType *controlParams, float maxPower) 
{
    DRV_CTRL_PARAMS = *controlParams;
    __HAL_TIM_SET_AUTORELOAD(DRV_ENCODER_SPEED_TIMER, controlParams->periodUs - 1);
    
    lastEncoderDiff = 0;
    errorIntegral = 0.0f;
    previousError = 0.0f;

    _drv_MotorInit();
    drv_SetMaxPower(maxPower);
    _drv_InitEncoder();

    int_SubscribeToInt(INT_TIM_PERIOD_ELAPSED, (int_CallbackFn)_drv_TimerInterruptHandler, NULL, DRV_ENCODER_SPEED_TIMER);
    HAL_TIM_Base_Start_IT(DRV_ENCODER_SPEED_TIMER);
}

void drv_Enable(bool enable) 
{
    if (enable) 
    {
        errorIntegral = 0.0f;
        previousError = 0.0f;
        lastEncoderDiff = 0;
    }
    _drv_EnableMotor(enable);
}

void drv_SetControlParams(const drv_ControlParamsType *controlParams) 
{
    DRV_CTRL_PARAMS = *controlParams;
    __HAL_TIM_SET_AUTORELOAD(DRV_ENCODER_SPEED_TIMER, controlParams->periodUs - 1);
}

void drv_GetAllParams(drv_ControlParamsType *controlParams, float *maxPower) 
{
    *controlParams = DRV_CTRL_PARAMS;
    _drv_GetMaxPower(maxPower);
}

int32_t drv_GetEncoderSpeed(void) 
{
    return (int32_t)(((int64_t)lastEncoderDiff * 1000000) / DRV_CTRL_PARAMS.periodUs);
}

