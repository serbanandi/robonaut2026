#ifndef SYS_MAIN_H
#define SYS_MAIN_H

#include <stdbool.h>
#include "spi.h"
#include "stm32n6xx_hal.h"
#include "stm32n6xx_nucleo.h"
#include "tim.h"

typedef enum
{
    SYS_STATE_NAVIGATE,
    SYS_STATE_FULL_BLACK_LINE_DETECTED,
    SYS_STATE_Y_TURN,
} sys_NavigationStateType;

#endif // SYS_MAIN_H
