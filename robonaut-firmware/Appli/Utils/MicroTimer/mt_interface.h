#ifndef MT_INTERFACE_H_
#define MT_INTERFACE_H_

#include <stdint.h>

void mt_Init();

/**
 * @brief Microsecond delay function
 * @param us Delay in microseconds
 * @warning DO NOT use for delays longer than ~60ms (timer overflow)
 */
void mt_Delay(uint16_t us);

/**
 * @brief Get the current microsecond tick count (max 65535 us)
 * @return Current tick count in microseconds
 */
uint16_t mt_GetTick(void);

#endif /* MT_INTERFACE_H_ */
