#include "../mt_interface.h"

#include "tim.h"

#define _MT_TIM_HANDLE &htim6

void mt_Init()
{
    HAL_TIM_Base_Start(_MT_TIM_HANDLE);
}

void mt_Delay(uint16_t delay_us)
{
    uint16_t tickstart = mt_GetTick();
    while ((uint16_t) (mt_GetTick() - tickstart) < delay_us)
        ;
}

uint16_t mt_GetTick(void)
{
    return __HAL_TIM_GET_COUNTER(_MT_TIM_HANDLE);
}
