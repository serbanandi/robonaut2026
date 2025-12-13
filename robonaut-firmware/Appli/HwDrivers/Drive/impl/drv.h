#ifndef DRV_H
#define DRV_H

#include "tim.h"

#define DRV_ENCODER_SPEED_TIMER &htim7  // Timer for speed calculation, 1 tick = 1us

void _drv_TimerInterruptHandler(void*);

#endif // DRV_H