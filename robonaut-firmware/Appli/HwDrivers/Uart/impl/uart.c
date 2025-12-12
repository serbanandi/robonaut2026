#include "../uart_interface.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "IntHandler/int_interface.h"


static void uart_HandleReceiveCplt(uart_UartType *uart)
{
	HAL_UART_Receive_DMA(uart->huart, (uint8_t*)uart->readCircularBuffer, UART_READ_BUFFER_LENGTH);
}

static void uart_HandleTransmitCplt(uart_UartType *uart)
{
	if(uart->startOfWriteData == -1)
    {
		uart->transmissionInProgress = 0;
		return;
	}
	int charCount;
	if(uart->startOfWriteData <= uart->endOfWriteData)
    {
		charCount = uart->endOfWriteData - uart->startOfWriteData + 1;
		HAL_UART_Transmit_DMA(uart->huart, (uint8_t*)uart->writeCircularBuffer + uart->startOfWriteData, charCount);
		uart->startOfWriteData = -1;
	}
	else
    {
		charCount = UART_WRITE_BUFFER_LENGTH - uart->startOfWriteData;
		HAL_UART_Transmit_DMA(uart->huart, (uint8_t*)uart->writeCircularBuffer + uart->startOfWriteData, charCount);
		uart->startOfWriteData = 0;
	}
}

uint8_t uart_Init(uart_UartType *uart, UART_HandleTypeDef *huart, IRQn_Type uartIr, uint8_t readTerminationChar)
{
	uart->huart = huart;
	uart->readPtr = 0;
	uart->uartIr = uartIr;
    uart->readTerminationChar = readTerminationChar;

	uart->startOfWriteData = -1;
	uart->endOfWriteData = UART_WRITE_BUFFER_LENGTH - 1;
	uart->transmissionInProgress = 0;

    uint8_t ok = int_SubscribeToInt(INT_UART_TX_CPLT, (int_CallbackFn)uart_HandleTransmitCplt, (void*)uart, (void*)huart);
    ok = ok && int_SubscribeToInt(INT_UART_RX_CPLT, (int_CallbackFn)uart_HandleReceiveCplt, (void*)uart, (void*)huart);
    __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF);
	ok = ok && HAL_UART_Receive_DMA(uart->huart, (uint8_t*)uart->readCircularBuffer, UART_READ_BUFFER_LENGTH) == HAL_OK;

    return ok;
}

uint8_t uart_Transmit(uart_UartType *uart, const uint8_t *str, const size_t size)
{
	if(size > UART_WRITE_BUFFER_LENGTH || size == 0)
		return 0;

    uint8_t ok = 1;

	size_t spaceTillBufferEnd = UART_WRITE_BUFFER_LENGTH - uart->endOfWriteData - 1;

	if(spaceTillBufferEnd >= size)
    {
		memcpy((void*)uart->writeCircularBuffer + uart->endOfWriteData + 1, (const void*)str, size);
		HAL_NVIC_DisableIRQ(uart->uartIr);
		if(uart->startOfWriteData == -1)
        {
			if(uart->transmissionInProgress)
            {
				uart->startOfWriteData = uart->endOfWriteData + 1;
			}
            else
            {
				ok = ok && HAL_UART_Transmit_DMA(uart->huart, (uint8_t*)uart->writeCircularBuffer + uart->endOfWriteData + 1, size) == HAL_OK;
				uart->transmissionInProgress = 1;
			}
		}
		uart->endOfWriteData = uart->endOfWriteData + size;
		HAL_NVIC_EnableIRQ(uart->uartIr);
	}
    else
    {
		if(spaceTillBufferEnd > 0)
			memcpy((void*)uart->writeCircularBuffer + uart->endOfWriteData + 1, (const void*)str, spaceTillBufferEnd);
		memcpy((void*)uart->writeCircularBuffer, (const void*)str + spaceTillBufferEnd, size - spaceTillBufferEnd);
		HAL_NVIC_DisableIRQ(uart->uartIr);
		if(uart->startOfWriteData == -1)
        {
			if(spaceTillBufferEnd == 0)
            {
				if(uart->transmissionInProgress)
                {
					uart->startOfWriteData = 0;
				}
                else
                {
					uart->transmissionInProgress = 1;
					ok = ok && HAL_UART_Transmit_DMA(uart->huart, (uint8_t*)uart->writeCircularBuffer, size) == HAL_OK;
				}
				uart->endOfWriteData = size - 1;
			}
            else
            {
				if(uart->transmissionInProgress)
                {
					uart->startOfWriteData = uart->endOfWriteData + 1;
				}
                else
                {
					uart->transmissionInProgress = 1;
					uart->startOfWriteData = 0;
					ok = ok && HAL_UART_Transmit_DMA(uart->huart, (uint8_t*)uart->writeCircularBuffer + uart->endOfWriteData + 1, spaceTillBufferEnd) == HAL_OK;
				}
				uart->endOfWriteData = size - spaceTillBufferEnd - 1;
			}
		}
        else
        {
			uart->endOfWriteData = size - spaceTillBufferEnd - 1;
		}
		HAL_NVIC_EnableIRQ(uart->uartIr);
	}
    return ok;
}

uint8_t uart_Receive(uart_UartType *uart, uint8_t* data , const size_t maxSize, size_t* receivedSize)
{
	uint16_t bytesLeft = (uint16_t)__HAL_DMA_GET_COUNTER(uart->huart->hdmarx);
    size_t endPtr = UART_READ_BUFFER_LENGTH - bytesLeft;
    size_t count = 0;
    while(uart->readPtr != endPtr && count < maxSize)
    {
		if (bytesLeft == 0 && uart->readPtr == 0)  // Handle if rx restart interrupt has not yet occurred
		{
			break;
		}
        data[count] = uart->readCircularBuffer[uart->readPtr];
        count++;
        uart->readPtr++;
        if (uart->readPtr >= UART_READ_BUFFER_LENGTH)
        {
            uart->readPtr = 0;
        }
        if(data[count - 1] == uart->readTerminationChar)
        {
            *receivedSize = count;
            return 1;
        }
    }
    *receivedSize = count;
    return 0;
}
