#ifndef UI_INTERFACE_H
#define UI_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    bool enterButtonWasPressed;
    bool backButtonWasPressed;
    bool knobButtonWasPressed;
} ui_StateType;

void ui_Init(void);

void ui_Process();

void ui_GetButtonState(ui_StateType* state);

int32_t ui_GetEncoderPosition();

#endif // UI_INTERFACE_H
