#ifndef RC_INTERFACE_H_
#define RC_INTERFACE_H_

#include <stdint.h>

// RX State Machine
typedef enum
{
    RC_RX_INACTIVE,
    RC_RX_COUNTDOWN_5,
    RC_RX_COUNTDOWN_4,
    RC_RX_COUNTDOWN_3,
    RC_RX_COUNTDOWN_2,
    RC_RX_COUNTDOWN_1,
    RC_RX_COUNTDOWN_0,
    RC_RX_POSITION,
} rc_RxStateType;

typedef struct
{
    uint8_t fromNode;
    uint8_t toNode;
    uint8_t nextNode;
    uint8_t positionPercent; // 0-100
} rc_PositionType;

/**
 * @brief Initialize the radio communication system.
 * @return 1 on success, 0 on failure.
 */
uint8_t rc_Init(void);

/**
 * @brief Process incoming and outgoing radio control data.
 */
void rc_Process(void);

/**
 * @brief Get the latest received RC position data.
 * @param position Pointer to store the position data.
 * @return 1 if new data is available, 0 otherwise.
 */
uint8_t rc_GetPosition(rc_PositionType* position);

/**
 * @brief Get the current RX state.
 * @return Current RX state.
 */
rc_RxStateType rc_GetRxState(void);

#endif // RC_INTERFACE_H_
