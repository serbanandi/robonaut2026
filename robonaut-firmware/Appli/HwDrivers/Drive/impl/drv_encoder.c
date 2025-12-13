#include "drv_encoder.h"

void _drv_InitEncoder(void) 
{
    HAL_TIM_Encoder_Start(DRV_ENCODER_TIMER, TIM_CHANNEL_ALL);
    __HAL_TIM_SET_COUNTER(DRV_ENCODER_TIMER, 0);
}

uint32_t drv_GetEncoderCount(void)
{
    return __HAL_TIM_GET_COUNTER(DRV_ENCODER_TIMER);
}
