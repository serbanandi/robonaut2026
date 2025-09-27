#ifndef ROBONAUT_MICROTIMER_MICROTIMER_H_
#define ROBONAUT_MICROTIMER_MICROTIMER_H_

#include <stdbool.h>
#include "tim.h"

bool MT_Init(TIM_HandleTypeDef* timer);
void MT_Delay(uint16_t us);
uint16_t MT_GetTick(void);


#endif /* ROBONAUT_MICROTIMER_MICROTIMER_H_ */
