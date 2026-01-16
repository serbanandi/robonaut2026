#ifndef DRV_INTERFACE_H
#define DRV_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

#define DRV_ENCODER_COUNTS_PER_M 70000 // Number of encoder counts per meter of travel
#define DRV_ENCODER_COUNTS_PER_CM (DRV_ENCODER_COUNTS_PER_M / 100)

// Control parameters structure
typedef struct
{
    float P, I, D;       // PID controller coefficients
    float integralLimit; // Integral windup limit
    uint32_t periodUs;   // Encoder speed calculation and control loop period in microseconds
} drv_ControlParamsType;

/**
 * @brief Initialize the drive module.
 * @param controlParams Pointer to the control parameters structure.
 * @param maxPower The maximum power level for the motor, in the range [0.0, 1.0].
 * @param maxEncoderCps The maximum encoder counts per second for speed calculations.
 */
void drv_Init(const drv_ControlParamsType* controlParams, float maxPower, uint32_t maxEncoderCps);

/**
 * @brief Enable or disable the drive system.
 * @param enable True to enable the drive system, false to disable.
 */
void drv_Enable(bool enable);

/**
 * @brief Set the motor target speed.
 * @param speed The speed to set, in the range [-1.0, 1.0].
 */
void drv_SetSpeed(float speed);

/**
 * @brief Set the maximum power level for the motor.
 * @param maxPower The maximum power level, in the range [0.0, 1.0].
 */
void drv_SetMaxPower(float maxPower);

/**
 * @brief Set new control parameters.
 * @param controlParams Pointer to the new control parameters structure.
 */
void drv_SetControlParams(const drv_ControlParamsType* controlParams);

/**
 * @brief Set the maximum encoder counts per second for speed calculations.
 * @param maxEncoderCps The maximum encoder counts per second.
 */
void drv_SetMaxEncoderCps(uint32_t maxEncoderCps);

/**
 * @brief Get the current encoder count.
 * @return The current encoder count.
 */
uint32_t drv_GetEncoderCount(void);

/**
 * @brief Get the current encoder speed.
 * @return The current encoder speed (adjusted according to maxEncoderCps) between [-1.0, 1.0].
 */
float drv_GetEncoderSpeed(void);

#endif // DRV_INTERFACE_H
