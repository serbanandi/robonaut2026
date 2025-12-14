#include "../drv_interface.h"
#include "drv_motor.h"
#include "drv_encoder.h"
#include "drv.h"
#include "IntHandler/int_interface.h"
#include <limits.h>

static drv_ControlParamsType _drv_controlParams;
static float _drv_maxEncoderCps;
static volatile float _drv_encoderMultConst;
static volatile float _drv_speedControlDt;
static volatile float _drv_speedControlDrInv;

static volatile float _drv_previousError;
static volatile float _drv_errorIntegral;
static volatile float _drv_targetSpeed;
static volatile bool _drv_enabled;

static volatile float _drv_lastMeasuredSpeed;

void _drv_TimerInterruptHandler(void*)
{
    uint32_t currentEncoderCount = drv_GetEncoderCount();
    static uint32_t lastEncoderCount = 0;
    int64_t encoderDiff = ((int64_t)currentEncoderCount - (int64_t)lastEncoderCount);
    // Handle encoder count overflow/underflow
    if (encoderDiff > 0x7FFFFFFFLL)
        encoderDiff -= 0x100000000LL;
    else if (encoderDiff < -0x7FFFFFFFLL)
        encoderDiff += 0x100000000LL;
    _drv_lastMeasuredSpeed = ((float)encoderDiff) * _drv_encoderMultConst;
    lastEncoderCount = currentEncoderCount;

    _drv_HandleSpeedControl();
}

void _drv_HandleSpeedControl() 
{
    if (fabs(_drv_targetSpeed) < 0.005f || !_drv_enabled)
    {
        _drv_SetPower(0.0f);
        _drv_previousError = 0.0f;
        _drv_errorIntegral = 0.0f;
        return;
    }

    float error = _drv_targetSpeed - _drv_lastMeasuredSpeed;

    // Proportional term
    float P_out = _drv_controlParams.P * error;

    // Integral term
    _drv_errorIntegral += error * _drv_speedControlDt;
    // Clamp integral to prevent windup
    if (_drv_errorIntegral > _drv_controlParams.integralLimit)
        _drv_errorIntegral = _drv_controlParams.integralLimit;
    else if (_drv_errorIntegral < -_drv_controlParams.integralLimit)
        _drv_errorIntegral = -_drv_controlParams.integralLimit;
    float I_out = _drv_controlParams.I * _drv_errorIntegral;

    // Derivative term
    float derivative = (error - _drv_previousError) * _drv_speedControlDrInv;
    float D_out = _drv_controlParams.D * derivative;

    // Compute total output
    float output = P_out + I_out + D_out;

    // Set motor power
    _drv_SetPower(output);

    _drv_previousError = error;
}

void drv_Init(const drv_ControlParamsType *controlParams, float maxPower, uint32_t maxEncoderCps) 
{
    _drv_enabled = false;

    drv_SetControlParams(controlParams);
    drv_SetMaxEncoderCps(maxEncoderCps);
    
    _drv_errorIntegral = 0.0f;
    _drv_previousError = 0.0f;

    _drv_MotorInit();
    drv_SetMaxPower(maxPower);
    _drv_InitEncoder();

    int_SubscribeToInt(INT_TIM_PERIOD_ELAPSED, (int_CallbackFn)_drv_TimerInterruptHandler, NULL, DRV_ENCODER_SPEED_TIMER);
    HAL_TIM_Base_Start_IT(DRV_ENCODER_SPEED_TIMER);
}

void drv_Enable(bool enable) 
{
    _drv_enabled = enable;
    _drv_EnableMotor(enable);
}

void drv_SetSpeed(float speed) 
{
    if (speed > 1.0f)
        speed = 1.0f;
    else if (speed < -1.0f)
        speed = -1.0f;
    _drv_targetSpeed = speed;
}

void drv_SetControlParams(const drv_ControlParamsType *controlParams) 
{
    _drv_controlParams = *controlParams;
    _drv_encoderMultConst = 1000000.0f / (float)_drv_controlParams.periodUs / _drv_maxEncoderCps;
    _drv_speedControlDt = (float)_drv_controlParams.periodUs * 0.000001f; // Convert microseconds to seconds
    _drv_speedControlDrInv = 1.0f / _drv_speedControlDt;
    __HAL_TIM_SET_AUTORELOAD(DRV_ENCODER_SPEED_TIMER, _drv_controlParams.periodUs - 1);
}

void drv_SetMaxEncoderCps(uint32_t maxEncoderCps) 
{
    _drv_maxEncoderCps = (float)maxEncoderCps;
    _drv_encoderMultConst = 1000000.0f / (float)_drv_controlParams.periodUs / _drv_maxEncoderCps;
}

float drv_GetEncoderSpeed(void) 
{
    return _drv_lastMeasuredSpeed;
}

