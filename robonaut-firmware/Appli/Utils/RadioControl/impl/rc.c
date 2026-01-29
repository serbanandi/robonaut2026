#include "rc.h"
#include <stdbool.h>
#include <stdint.h>
#include "../rc_interface.h"
#include "Uart/uart_interface.h"
#include "usart.h"

// --- Static Variables ---
static uart_UartType rc_uart __NON_CACHEABLE;

static rc_RxStateType rxState = RC_RX_INACTIVE;
static uint8_t rxBuffer[RC_RX_BUFFER_SIZE]; // Payload buffer
static rc_PositionType latestPosition __NON_CACHEABLE;
static uint8_t rxByteCnt = 0;
static bool newPosition = false;

/**
 * The radio module sends ASCII messages to the main controller to manage the
 * start of the skill run and to provide periodic updates on the pirate's position.
 * Before the skill run begins, the radio module receives a countdown:
 *              ’5\r’, ’4\r’, ’3\r’, ’2\r’, ’1\r’, ’0\r’,
 * The car can only start moving after receiving the '0' message.
 *
 * After the start of the skill run, the radio module periodically sends the
 * position of the pirate robot: the letters of the nodes it is currently moving
 * between (X,Y), the letter of the node it will head to next (Z),
 * and a percentage indicating its exact position between those nodes (nnn).
 * The format is:
 *              ’XYZnnn\r’
 * The position is sent every 0.2 seconds. If the skill run has not started yet,
 * or if the allocated time has expired, the transmitter goes into an inactive
 * state and does not send any data.
 */

// --- Helper Functions ---
/**
 * @brief Calculate the latest position from the received buffer.
 * This function assumes that the rxBuffer contains a valid position message.
 */
static void rc_CalculatePosition(void)
{
    // validate data before updating latestPosition
    if (rxBuffer[0] < 'A' || rxBuffer[0] > 'Z')
        return;
    if (rxBuffer[1] < 'A' || rxBuffer[1] > 'Z')
        return;
    if (rxBuffer[2] < 'A' || rxBuffer[2] > 'Z')
        return;
    if (rxBuffer[3] < '0' || rxBuffer[3] > '9')
        return;
    if (rxBuffer[4] < '0' || rxBuffer[4] > '9')
        return;
    if (rxBuffer[5] < '0' || rxBuffer[5] > '9')
        return;

    latestPosition.fromNode = rxBuffer[0];
    latestPosition.toNode = rxBuffer[1];
    latestPosition.nextNode = rxBuffer[2];

    // Parse position percentage (3 digits)
    uint8_t hundreds = rxBuffer[3] - '0';
    uint8_t tens = rxBuffer[4] - '0';
    uint8_t units = rxBuffer[5] - '0';

    latestPosition.positionPercent = hundreds * 100 + tens * 10 + units;
    newPosition = true;
}

/**
 * @brief Parse a received byte from the radio module.
 * @param byte Received byte.
 */
static void rc_ParseRxByte(uint8_t byte)
{
    switch (rxState)
    {
        case RC_RX_INACTIVE:
            // Look for countdown messages
            if (byte >= '0' && byte <= '5')
            {
                rxState = (rc_RxStateType) (RC_RX_COUNTDOWN_5 + ('5' - byte));
            }
            return;
        case RC_RX_COUNTDOWN_5:
        case RC_RX_COUNTDOWN_4:
        case RC_RX_COUNTDOWN_3:
        case RC_RX_COUNTDOWN_2:
        case RC_RX_COUNTDOWN_1:
            // Look for the next countdown or '0'
            if (byte >= '0' && byte <= '5')
            {
                rxState = (rc_RxStateType) (RC_RX_COUNTDOWN_5 + ('5' - byte));
            }
            return;
        case RC_RX_COUNTDOWN_0:
        case RC_RX_POSITION:
            // Collect position data
            if (rxByteCnt < RC_RX_BUFFER_SIZE)
            {
                rxBuffer[rxByteCnt++] = byte; // Store byte
                if (byte == '\r')
                {
                    // End of message, process the received data
                    if (rxByteCnt == RC_RX_BUFFER_SIZE)
                    {
                        // Full position message received
                        rxState = RC_RX_POSITION;
                        rc_CalculatePosition();
                    }
                    rxByteCnt = 0; // Reset for next message
                }
            }
            else
            {
                // Buffer overflow, reset state
                rxByteCnt = 0;
                rxState = RC_RX_POSITION;
            }
            break;
        default: break;
    }
}

// --- Implementation ---
uint8_t rc_Init(void)
{
    rxState = RC_RX_INACTIVE;
    latestPosition = (rc_PositionType) { 0 };
    newPosition = false;
    return uart_Init(&rc_uart, &huart8, UART8_IRQn, '\r');
}

void rc_Process(void)
{
    // Handle Incoming Data
    // Directly accessing buffer to avoid blocking behavior of uart_Receive
    // or logic issues with termination chars in binary data.

    // We fetch one byte at a time from the UART circular buffer
    uint8_t byte;
    // Store last time data was received
    static uint32_t lastRxTime = 0;

    // We loop as long as there is data
    while (1)
    {
        // We use the driver's receive but with max size 1.
        // Note: The driver provided relies on finding a termination char to return 1.
        // This is problematic for binary streams.
        // We will manually pull from the circular buffer structure provided in the header.

        uint32_t bytesLeft = (uint16_t) __HAL_DMA_GET_COUNTER(rc_uart.huart->hdmarx);

        if (rc_uart.readPtr == (UART_READ_BUFFER_LENGTH - bytesLeft) || (bytesLeft == 0 && rc_uart.readPtr == 0))
        {
            break; // Buffer empty (pointers match)
        }

        lastRxTime = HAL_GetTick();
        byte = rc_uart.readCircularBuffer[rc_uart.readPtr];
        rc_uart.readPtr++;
        if (rc_uart.readPtr >= UART_READ_BUFFER_LENGTH)
        {
            rc_uart.readPtr = 0;
        }

        rc_ParseRxByte(byte);
    }

    // States RC_RX_COUNTDOWN_5-2: considered inactive after 1 second without new data
    // State RC_RX_COUNTDOWN_1: considered inactive after 4 seconds without new data
    // State RC_RX_COUNTDOWN_0, RC_RX_POSITION: considered inactive after 0.2 seconds without new data
    switch (rxState)
    {
        case RC_RX_COUNTDOWN_5:
        case RC_RX_COUNTDOWN_4:
        case RC_RX_COUNTDOWN_3:
        case RC_RX_COUNTDOWN_2:
            if (HAL_GetTick() - lastRxTime > 1100)
                rxState = RC_RX_INACTIVE;
            break;
        case RC_RX_COUNTDOWN_1:
            if (HAL_GetTick() - lastRxTime > 4100)
                rxState = RC_RX_INACTIVE;
            break;
        case RC_RX_COUNTDOWN_0:
        case RC_RX_POSITION:
            if (HAL_GetTick() - lastRxTime > 300)
                rxState = RC_RX_INACTIVE;
            break;
        default: break;
    }
}

uint8_t rc_GetPosition(rc_PositionType* position)
{
    if (newPosition)
    {
        *position = latestPosition;
        newPosition = false;
        return 1;
    }
    return 0;
}

rc_RxStateType rc_GetRxState(void)
{
    return rxState;
}
