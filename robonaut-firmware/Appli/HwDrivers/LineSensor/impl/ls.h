#ifndef LS_H_
#define LS_H_

typedef enum
{
    _LS_CS_NONE = 0,
    _LS_CS_AD1,
    _LS_CS_AD2,
    _LS_CS_AD3,
    _LS_CS_AD4
} _ls_CSType;

typedef enum
{
    _LS_DISABLE = 0,
    _LS_ENABLE = 1
} _ls_EnableType;

typedef enum
{
    _LS_STATE_IDLE = 0,
    _LS_STATE_SET_LEDS,
    _LS_STATE_WAIT,
} _ls_StateType;

#define _LS_MAX_ADC_VAL (4095)

#define _LS_FRONT_SENSOR_SPI &hspi4
#define _LS_REAR_SENSOR_SPI &hspi5
#define _LS_SPI_TIMEOUT_MS (2)

#endif // LS_H_
