#include "MicroTimer.h"

static TIM_HandleTypeDef* tim_handle;

bool MT_Init(TIM_HandleTypeDef* timer)
{
	assert(timer != NULL);
	tim_handle = timer;
	return HAL_TIM_Base_Start(timer) == HAL_OK;
}

void MT_Delay(uint16_t delay_us)
{
	uint16_t tickstart = MT_GetTick();
	while ((uint16_t)(MT_GetTick() - tickstart) < delay_us);
}

uint16_t MT_GetTick(void)
{
	return __HAL_TIM_GET_COUNTER(tim_handle);
}
