#ifndef MOT_INTERFACE_H
#define MOT_INTERFACE_H

#include <stdbool.h>

void mot_Init(void);

void mot_Enable(bool enable);

void mot_SetSpeed(float speed);

#endif // MOT_INTERFACE_H