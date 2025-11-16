#ifndef SERVO_INTERFACE_H
#define SERVO_INTERFACE_H

#include <stdint.h>

typedef enum {
    SERVO_FRONT = 0,
    SERVO_BACK,
} servo_SelectType;

void servo_Init(void);

void servo_SetAngle(servo_SelectType servo, int16_t pos);

#endif // SERVO_INTERFACE_H