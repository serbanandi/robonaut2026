#include "servo.h"
#include <stdint.h>

void servo_Init(void)
{
    servo_SetAngle(SERVO_FRONT, 0.0f);
    servo_SetAngle(SERVO_BACK, 0.0f);
    HAL_TIM_PWM_Start(SERVO_TIMER, SERVO_FRONT_CHANNEL);
    HAL_TIM_PWM_Start(SERVO_TIMER, SERVO_BACK_CHANNEL);
}

/**
 * @brief Set the position of the specified servo.
 * @param servo The servo to set (SERVO_FRONT or SERVO_BACK).
 * @param pos The position to set, in the range [-1.0, 1.0].
 */
void servo_SetAngle(servo_SelectType servo, float pos)
{
    if (pos < -1.0f)
        pos = -1.0f;
    if (pos > 1.0f)
        pos = 1.0f;

    uint32_t pulseWidth =
        (uint32_t) ((((pos + 1.0f) / 2.0f) * (SERVO_MAX_PULSE_WIDTH - SERVO_MIN_PULSE_WIDTH)) + SERVO_MIN_PULSE_WIDTH);

    uint32_t channel;
    switch (servo)
    {
        case SERVO_FRONT: channel = SERVO_FRONT_CHANNEL; break;
        case SERVO_BACK: channel = SERVO_BACK_CHANNEL; break;
        default: return; // Invalid servo selection
    }

    __HAL_TIM_SET_COMPARE(SERVO_TIMER, channel, pulseWidth);
}
