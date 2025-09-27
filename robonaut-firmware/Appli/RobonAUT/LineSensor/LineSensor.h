#ifndef LINESENSOR_H_
#define LINESENSOR_H_

#include <stdbool.h>

bool LS_Init(SPI_HandleTypeDef* front_spi, SPI_HandleTypeDef* rear_spi);
void LS_Process(void);

#endif /* LINESENSOR_H_ */
