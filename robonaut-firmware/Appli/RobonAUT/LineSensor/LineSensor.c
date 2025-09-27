#include "main.h"
#include "LineSensor.h"
#include "MicroTimer/MicroTimer.h"
#include "SimpleLogger/SimpleLogger.h"

// Types
typedef enum {
	NONE = 0, AD1, AD2, AD3, AD4
} ADC_CS_Type;

typedef enum {
	Disable = 0, Enable = 1
} LS_Enable_Type;

typedef struct
{
	SPI_HandleTypeDef* spi;
	uint16_t adc[32];
} LS_Sensor_Type;

typedef enum {
    LS_STATE_IDLE = 0,
    LS_STATE_WAIT,
    LS_STATE_READ
} LS_State_Type;

// Defines
#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))
#define SPI_TIMEOUT_MS	(10)
#define LS_PERIOD_MS (5)
#define LS_MAX_ADC_VAL (4095)

// Variables
static LS_Sensor_Type front_sensor;
static LS_Sensor_Type rear_sensor;
static LS_State_Type state = LS_STATE_IDLE;
static uint32_t last_read_tick;

// Static helper functions
static void LS_ADC_SetCS(ADC_CS_Type cs)
{
	switch (cs)
	{
	case NONE:
	default:
		HAL_GPIO_WritePin(_AD_CS1_GPIO_Port, _AD_CS1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(_AD_CS2_GPIO_Port, _AD_CS2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(_AD_CS3_GPIO_Port, _AD_CS3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(_AD_CS4_GPIO_Port, _AD_CS4_Pin, GPIO_PIN_SET);
		break;

	case AD1:
		HAL_GPIO_WritePin(_AD_CS1_GPIO_Port, _AD_CS1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(_AD_CS2_GPIO_Port, _AD_CS2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(_AD_CS3_GPIO_Port, _AD_CS3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(_AD_CS4_GPIO_Port, _AD_CS4_Pin, GPIO_PIN_SET);
		break;

	case AD2:
		HAL_GPIO_WritePin(_AD_CS1_GPIO_Port, _AD_CS1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(_AD_CS2_GPIO_Port, _AD_CS2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(_AD_CS3_GPIO_Port, _AD_CS3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(_AD_CS4_GPIO_Port, _AD_CS4_Pin, GPIO_PIN_SET);
		break;

	case AD3:
		HAL_GPIO_WritePin(_AD_CS1_GPIO_Port, _AD_CS1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(_AD_CS2_GPIO_Port, _AD_CS2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(_AD_CS3_GPIO_Port, _AD_CS3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(_AD_CS4_GPIO_Port, _AD_CS4_Pin, GPIO_PIN_SET);
		break;

	case AD4:
		HAL_GPIO_WritePin(_AD_CS1_GPIO_Port, _AD_CS1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(_AD_CS2_GPIO_Port, _AD_CS2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(_AD_CS3_GPIO_Port, _AD_CS3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(_AD_CS4_GPIO_Port, _AD_CS4_Pin, GPIO_PIN_RESET);
		break;
	}
}

static void LS_IR_SetLatch(LS_Enable_Type enable)
{
	switch (enable) {
	case Enable:
		HAL_GPIO_WritePin(LINE_INF_LE_GPIO_Port, LINE_INF_LE_Pin, GPIO_PIN_SET);
		MT_Delay(1);
		HAL_GPIO_WritePin(LINE_INF_LE_GPIO_Port, LINE_INF_LE_Pin, GPIO_PIN_RESET);
		break;

	case Disable:
	default:
		HAL_GPIO_WritePin(LINE_INF_LE_GPIO_Port, LINE_INF_LE_Pin, GPIO_PIN_RESET);
		break;
	}
}

static void LS_IR_SetOutput(LS_Enable_Type enable)
{
	switch (enable) {
	case Enable:
		HAL_GPIO_WritePin(_LINE_INF_OE_GPIO_Port, _LINE_INF_OE_Pin, GPIO_PIN_RESET);
		break;

	case Disable:
	default:
		HAL_GPIO_WritePin(_LINE_INF_OE_GPIO_Port, _LINE_INF_OE_Pin, GPIO_PIN_SET);
		break;
	}
}

static bool LS_IR_SetLED(LS_Sensor_Type* sensor, uint8_t act_led)
{
	assert(sensor != NULL);

	if (sensor->spi == NULL)
		return true; 	//Sensor disabled

	uint8_t cmd = 0x01 << act_led;
	if (HAL_SPI_Transmit(sensor->spi, &cmd, sizeof(cmd), SPI_TIMEOUT_MS) != HAL_OK)
		return false;

	return true;
}

static bool LS_ADC_Read(LS_Sensor_Type* sensor, uint8_t act_sens, uint8_t block_num)
{
	assert(sensor != NULL);

	if (sensor->spi == NULL)
		return true; 	//Sensor disabled

	uint8_t cmd[2] = {0};
	cmd[0] = act_sens << 3;

	uint16_t* adc_data = &sensor->adc[(block_num - 1) * 8 + act_sens];

	if (HAL_SPI_Transmit(sensor->spi, cmd, 2, SPI_TIMEOUT_MS) != HAL_OK)	//Set active channel
		return false;

	if (HAL_SPI_Receive(sensor->spi, (uint8_t*)adc_data, 2, SPI_TIMEOUT_MS) != HAL_OK)	//Read adc value
		return false;

	*adc_data = __REVSH(*adc_data);
	return true;
}

static bool LS_ADC_Valdidate(LS_Sensor_Type* sensor)
{
	assert(sensor != NULL);

	if (sensor->spi == NULL)
		return true; 	//Sensor disabled

	for (uint32_t i = 0; i < ARRAY_LEN(sensor->adc); ++i)
	{
		if (sensor->adc[i] > LS_MAX_ADC_VAL)
		{
			Log("LS", "raw ADC data not valid for %d channel", i);
			return false;
		}
	}

	return true;
}

static bool LS_Read(void)
{
	Log("LS", "read cycle start");
	for (int i = 0; i < 8; i++)
	{
		//Light up IR LEDs
		for (int block_num = 1; block_num <= 4; block_num++)
		{
			if (!LS_IR_SetLED(&front_sensor, i))
				return false;
			if (!LS_IR_SetLED(&rear_sensor, i))
				return false;
		}

		LS_IR_SetLatch(Enable);

		//Wait for LEDs, sensors
		MT_Delay(180);

		//Read sensors
		for (int block_num = 1; block_num <= 4; block_num++)
		{
			LS_ADC_SetCS(block_num);

			if (!LS_ADC_Read(&front_sensor, i, block_num))
				return false;

			if (!LS_ADC_Read(&rear_sensor, i, block_num))
				return false;
		}

		LS_ADC_SetCS(NONE);
	}

	if (!LS_ADC_Valdidate(&front_sensor))
		return false;
	if (!LS_ADC_Valdidate(&rear_sensor))
		return false;

	Log("LS", "read cycle done");
	return true;
}

// Public functions
bool LS_Init(SPI_HandleTypeDef* front_spi, SPI_HandleTypeDef* rear_spi)
{
	front_sensor.spi = front_spi;
	rear_sensor.spi = rear_spi;

	Log("LS", "init done, front sensor: %d, rear sensor: %d", front_spi != NULL, rear_spi != NULL);
	return true;
}

void LS_Process(void)
{
    switch (state)
    {
    case LS_STATE_IDLE:
    	LS_ADC_SetCS(NONE);
    	LS_IR_SetLatch(Disable);
    	LS_IR_SetOutput(Enable);

    	last_read_tick = HAL_GetTick();
    	state = LS_STATE_WAIT;
        break;

    case LS_STATE_WAIT:
    	if ((HAL_GetTick() - last_read_tick) >= LS_PERIOD_MS)
    		state = LS_STATE_READ;
        break;

    case LS_STATE_READ:
    	last_read_tick = HAL_GetTick();

        if (!LS_Read()) {
            Log("LS", "read failed");
        }

        state = LS_STATE_WAIT;
        break;

    default:
        state = LS_STATE_IDLE;
        break;
    }
}
