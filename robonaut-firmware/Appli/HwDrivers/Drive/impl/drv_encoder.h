#ifndef DRV_ENCODER_H
#define DRV_ENCODER_H

#include <stdint.h>
#include "tim.h"

#define DRV_ENCODER_TIMER &htim4

void _drv_InitEncoder(void);

uint32_t drv_GetEncoderCount(void);

#endif // DRV_ENCODER_H
