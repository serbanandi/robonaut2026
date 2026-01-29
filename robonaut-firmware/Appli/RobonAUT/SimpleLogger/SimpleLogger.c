#include "SimpleLogger.h"
#include <stdarg.h>
#include <stdio.h>
#include "MicroTimer/mt_interface.h"
#include "stm32n6xx_hal.h"

void Log(const char* tag, const char* str, ...)
{
    va_list args;
    va_start(args, str);

    uint32_t time_ms = HAL_GetTick();
    uint16_t time_us = mt_GetTick();

    printf("[%s][%*lu][%*u] ", tag, 8, time_ms, 5, time_us);
    vprintf(str, args);

    printf("\r\n");
    va_end(args);
}
