#include "../lc_interface.h"
#include "main.h"
#include "math.h"

#define SETPOINT 15.5f
#define MAX_JUMP_THRESHOLD 5.0f // Ignore derivative spikes larger than this

static float last_pos, int_error;

void lc_Init()
{
    last_pos = 0;
    int_error = 0;
}

float lc_Compute(float pos, float P, float I, float D, float Ts)
{
    float control_signal;
    float error;
    float d_input;

    // 1. Calculate Error (Standard P-Term)
    error = pos - SETPOINT;

    // 2. Derivative on Measurement (Prevents kicks on setpoint change)
    // d_input = change in position, not change in error
    float delta_pos = (pos - last_pos);

    // 3. Transient Suppression
    // If we jumped too far in one cycle (e.g. switching lines), ignore the D term
    if (fabs(delta_pos) > MAX_JUMP_THRESHOLD)
    {
        d_input = 0.0f;
    }
    else
    {
        d_input = delta_pos / Ts;
    }

    // 4. Update Integral
    int_error += error * Ts;

    // 5. Calculate Output
    // Note: D term is subtracted because it opposes change
    control_signal = (P * error) + (I * int_error) - (D * d_input);

    last_pos = pos; // Save for next cycle

    return control_signal;
}
