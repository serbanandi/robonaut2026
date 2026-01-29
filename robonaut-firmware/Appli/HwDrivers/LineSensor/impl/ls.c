#include "ls.h"
#include "../ls_interface.h"
#include "MicroTimer/mt_interface.h"
#include "spi.h"

static SPI_HandleTypeDef* const _ls_spiHandles[2] = { _LS_FRONT_SENSOR_SPI, _LS_REAR_SENSOR_SPI };

static ls_AdcValuesType _ls_lastAdcValues[2]; // 0: front, 1: rear
static _ls_StateType _ls_state = _LS_STATE_IDLE;
static uint32_t _ls_readIndex = 0;
static uint16_t _ls_waitStartUs = 0;
static bool _ls_newDataAvailable = false;

static void _ls_ADCSetCS(_ls_CSType cs)
{
    switch (cs)
    {
        case _LS_CS_NONE:
        default:
            HAL_GPIO_WritePin(_AD_CS1_GPIO_Port, _AD_CS1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(_AD_CS2_GPIO_Port, _AD_CS2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(_AD_CS3_GPIO_Port, _AD_CS3_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(_AD_CS4_GPIO_Port, _AD_CS4_Pin, GPIO_PIN_SET);
            break;

        case _LS_CS_AD1:
            HAL_GPIO_WritePin(_AD_CS1_GPIO_Port, _AD_CS1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(_AD_CS2_GPIO_Port, _AD_CS2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(_AD_CS3_GPIO_Port, _AD_CS3_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(_AD_CS4_GPIO_Port, _AD_CS4_Pin, GPIO_PIN_SET);
            break;

        case _LS_CS_AD2:
            HAL_GPIO_WritePin(_AD_CS1_GPIO_Port, _AD_CS1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(_AD_CS2_GPIO_Port, _AD_CS2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(_AD_CS3_GPIO_Port, _AD_CS3_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(_AD_CS4_GPIO_Port, _AD_CS4_Pin, GPIO_PIN_SET);
            break;

        case _LS_CS_AD3:
            HAL_GPIO_WritePin(_AD_CS1_GPIO_Port, _AD_CS1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(_AD_CS2_GPIO_Port, _AD_CS2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(_AD_CS3_GPIO_Port, _AD_CS3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(_AD_CS4_GPIO_Port, _AD_CS4_Pin, GPIO_PIN_SET);
            break;

        case _LS_CS_AD4:
            HAL_GPIO_WritePin(_AD_CS1_GPIO_Port, _AD_CS1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(_AD_CS2_GPIO_Port, _AD_CS2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(_AD_CS3_GPIO_Port, _AD_CS3_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(_AD_CS4_GPIO_Port, _AD_CS4_Pin, GPIO_PIN_RESET);
            break;
    }
}

static void _ls_IrSetLatch(_ls_EnableType enable)
{
    switch (enable)
    {
        case _LS_ENABLE:
            HAL_GPIO_WritePin(LINE_INF_LE_GPIO_Port, LINE_INF_LE_Pin, GPIO_PIN_SET);
            mt_Delay(1);
            HAL_GPIO_WritePin(LINE_INF_LE_GPIO_Port, LINE_INF_LE_Pin, GPIO_PIN_RESET);
            break;

        case _LS_DISABLE:
        default: HAL_GPIO_WritePin(LINE_INF_LE_GPIO_Port, LINE_INF_LE_Pin, GPIO_PIN_RESET); break;
    }
}

static void _ls_IrSetOutput(_ls_EnableType enable)
{
    switch (enable)
    {
        case _LS_ENABLE: HAL_GPIO_WritePin(_LINE_INF_OE_GPIO_Port, _LINE_INF_OE_Pin, GPIO_PIN_RESET); break;
        case _LS_DISABLE:
        default: HAL_GPIO_WritePin(_LINE_INF_OE_GPIO_Port, _LINE_INF_OE_Pin, GPIO_PIN_SET); break;
    }
}

static bool _ls_IrSetLED(uint8_t act_led)
{
    uint32_t cmd = (1 << act_led) | (1 << (act_led + 8)) | (1 << (act_led + 16)) | (1 << (act_led + 24));

    if (HAL_SPI_Transmit(_LS_FRONT_SENSOR_SPI, (uint8_t*) &cmd, sizeof(cmd), _LS_SPI_TIMEOUT_MS) != HAL_OK ||
        HAL_SPI_Transmit(_LS_REAR_SENSOR_SPI, (uint8_t*) &cmd, sizeof(cmd), _LS_SPI_TIMEOUT_MS) != HAL_OK)
        return false;

    return true;
}

static void _ls_FbSetLatch(_ls_EnableType enable)
{
    switch (enable)
    {
        case _LS_ENABLE:
            HAL_GPIO_WritePin(LINE_LED_LE_GPIO_Port, LINE_LED_LE_Pin, GPIO_PIN_SET);
            mt_Delay(1);
            HAL_GPIO_WritePin(LINE_LED_LE_GPIO_Port, LINE_LED_LE_Pin, GPIO_PIN_RESET);
            break;

        case _LS_DISABLE:
        default: HAL_GPIO_WritePin(LINE_LED_LE_GPIO_Port, LINE_LED_LE_Pin, GPIO_PIN_RESET); break;
    }
}

static void _ls_FbSetOutput(_ls_EnableType enable)
{
    switch (enable)
    {
        case _LS_ENABLE: HAL_GPIO_WritePin(_LINE_LED_OE_GPIO_Port, _LINE_LED_OE_Pin, GPIO_PIN_RESET); break;
        case _LS_DISABLE:
        default: HAL_GPIO_WritePin(_LINE_LED_OE_GPIO_Port, _LINE_LED_OE_Pin, GPIO_PIN_SET); break;
    }
}

static bool _ls_ReadADC(uint8_t act_sens, uint8_t block_num, const ls_SensorPositionType sensor)
{
    uint8_t cmd[2] = { 0 };
    cmd[0] = act_sens << 3;

    uint16_t* adc_data = &_ls_lastAdcValues[sensor].v[(block_num - 1) * 8 + act_sens];

    if (HAL_SPI_Transmit(_ls_spiHandles[sensor], cmd, 2, _LS_SPI_TIMEOUT_MS) != HAL_OK) // Set active channel
        return false;

    if (HAL_SPI_Receive(_ls_spiHandles[sensor], (uint8_t*) adc_data, 2, _LS_SPI_TIMEOUT_MS) != HAL_OK) // Read adc value
        return false;

    *adc_data = __REVSH(*adc_data);
    return true;
}

static bool _ls_ValidateADC(ls_SensorPositionType sensor)
{
    for (uint32_t i = 0; i < 32; ++i)
    {
        if (_ls_lastAdcValues[sensor].v[i] > _LS_MAX_ADC_VAL)
            return false;
    }
    return true;
}

bool ls_Init(void)
{
    _ls_ADCSetCS(_LS_CS_NONE);
    _ls_IrSetLatch(_LS_DISABLE);
    _ls_IrSetOutput(_LS_DISABLE);
    _ls_waitStartUs = mt_GetTick();
    _ls_state = _LS_STATE_IDLE;
    _ls_readIndex = 0;
    return true;
}

bool ls_SetFbLEDs(const ls_LedValuesType* led_values, ls_SensorPositionType sensor)
{
    uint32_t cmd = 0;
    int q = 0;
    for (int i = 0; i < 4; i++)
    {
        uint8_t byte = 0;
        for (int bit = 0; bit < 8; bit++)
        {
            byte = led_values->v[q++] ? (byte >> 1) | 0x80 : (byte >> 1);
        }
        cmd = (cmd << 8) | byte;
    }

    if (HAL_SPI_Transmit(_ls_spiHandles[sensor], (uint8_t*) &cmd, sizeof(cmd), _LS_SPI_TIMEOUT_MS) != HAL_OK)
        return false;

    _ls_FbSetLatch(_LS_ENABLE);
    _ls_FbSetOutput(_LS_ENABLE);

    return true;
}

void ls_GetADCValues(ls_AdcValuesType* adc_values, const ls_SensorPositionType sensor)
{
    if (adc_values == NULL)
        return;

    *adc_values = _ls_lastAdcValues[sensor];
}

bool ls_Process()
{
    switch (_ls_state)
    {
        case _LS_STATE_IDLE:
        {
            _ls_ADCSetCS(_LS_CS_NONE);
            _ls_IrSetLatch(_LS_DISABLE);

            _ls_state = _LS_STATE_SET_LEDS;
            _ls_readIndex = 0;
            break;
        }

        case _LS_STATE_SET_LEDS:
        {
            _ls_IrSetLED(_ls_readIndex);
            _ls_IrSetLatch(_LS_ENABLE);
            _ls_IrSetOutput(_LS_ENABLE);
            _ls_state = _LS_STATE_WAIT;
            _ls_waitStartUs = mt_GetTick();
            break;
        }

        case _LS_STATE_WAIT:
        {
            uint16_t currentTimeUs = mt_GetTick();
            if ((uint16_t) (currentTimeUs - _ls_waitStartUs) >= 180)
            {
                for (int block_num = 1; block_num <= 4; block_num++)
                {
                    _ls_ADCSetCS(block_num);
                    _ls_ReadADC(_ls_readIndex, block_num, LS_SENSOR_FRONT);
                    _ls_ReadADC(_ls_readIndex, block_num, LS_SENSOR_REAR);
                }

                _ls_ADCSetCS(_LS_CS_NONE);

                _ls_readIndex++;
                if (_ls_readIndex >= 8)
                {
                    _ls_state = _LS_STATE_IDLE;
                    _ls_newDataAvailable = true;
                    bool adcFrontValid = _ls_ValidateADC(LS_SENSOR_FRONT);
                    bool adcRearValid = _ls_ValidateADC(LS_SENSOR_REAR);
                    return adcFrontValid && adcRearValid;
                }
                else
                {
                    _ls_state = _LS_STATE_SET_LEDS;
                }
            }
            break;
        }
    }
    return true;
}

bool ls_IsNewDataAvailable()
{
    return _ls_newDataAvailable;
}

void ls_ClearNewDataFlag()
{
    _ls_newDataAvailable = false;
}
