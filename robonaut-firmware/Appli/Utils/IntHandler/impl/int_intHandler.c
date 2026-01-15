#include "int_list.h"
#include "stm32n6xx_hal.h"

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    void* tmp = (void*) huart;
    _int_triggerCbs(INT_UART_RX_CPLT, tmp);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {
    void* tmp = (void*) huart;
    _int_triggerCbs(INT_UART_TX_CPLT, tmp);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t pos) {
    UNUSED(pos);
    void* tmp = (void*) huart;
    _int_triggerCbs(INT_UART_RX_EVENT, tmp);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart) {
    void* tmp = (void*) huart;
    _int_triggerCbs(INT_UART_ERROR, tmp);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi) {
    void* tmp = (void*) hspi;
    _int_triggerCbs(INT_SPI_TX_CPLT, tmp);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef* hspi) {
    void* tmp = (void*) hspi;
    _int_triggerCbs(INT_SPI_RX_CPLT, tmp);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
    void* tmp = (void*) htim;
    _int_triggerCbs(INT_TIM_PERIOD_ELAPSED, tmp);
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	void* tmp = (void*) hi2c;
    _int_triggerCbs(INT_I2C_MEM_TX_CPLT, tmp);
}
