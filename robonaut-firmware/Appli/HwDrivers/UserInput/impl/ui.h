#ifndef UI_H
#define UI_H

#include "tim.h"

#define UI_REMOTE_CONTROL_TIMER &htim13 // Timer used for remote control PWM input capture, 1 tick = 1 us

void _ui_TimerCaptureCallback(void*);

#endif // UI_H
