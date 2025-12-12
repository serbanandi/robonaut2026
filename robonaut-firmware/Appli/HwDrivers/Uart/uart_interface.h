#ifndef UART_INTERFACE_H_
#define UART_INTERFACE_H_

#include "stm32n6xx_hal.h"

#define UART_WRITE_BUFFER_LENGTH	2500
#define UART_READ_BUFFER_LENGTH		1000

typedef struct {
	UART_HandleTypeDef* huart;
	IRQn_Type uartIr;

	volatile uint8_t writeCircularBuffer[UART_WRITE_BUFFER_LENGTH];
	volatile int32_t startOfWriteData;
	volatile int32_t endOfWriteData;
	volatile uint8_t transmissionInProgress;

	volatile uint8_t readCircularBuffer[UART_READ_BUFFER_LENGTH];
	uint16_t readPtr;
    uint8_t readTerminationChar;
} uart_UartType;

/**
 * @brief Initialize the UART interface
 * @param uart Pointer to the UART interface structure
 * @param huart Pointer to the HAL UART handle
 * @param uartIr UART interrupt number
 * @param readTerminationChar Character that indicates the end of a read operation
 * @return 1 on success, 0 on failure
 */
uint8_t uart_Init(uart_UartType* uart, UART_HandleTypeDef *huart, IRQn_Type uartIr, uint8_t readTerminationChar);

/**
 * @brief Transmit data over UART
 * @param uart Pointer to the UART interface structure
 * @param str Pointer to the data to be transmitted
 * @param size Size of the data to be transmitted
 * @return 1 on success, 0 on failure
 */
uint8_t uart_Transmit(uart_UartType* uart, const uint8_t *str, const size_t size);

/**
 * @brief Receive data from UART
 * @param uart Pointer to the UART interface structure
 * @param data Pointer to the buffer where received data will be stored
 * @param maxSize Maximum size of the buffer
 * @param receivedSize Pointer to variable where the actual size of received data will be stored
 * @return 1 in case of success and if the end character is received is within maxSize, 0 otherwise
 */
uint8_t uart_Receive(uart_UartType* uart, uint8_t* data, const size_t maxSize, size_t* receivedSize);

#endif /* UART_UART_H_ */
