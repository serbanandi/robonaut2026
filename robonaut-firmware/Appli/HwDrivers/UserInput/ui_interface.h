#ifndef UI_INTERFACE_H
#define UI_INTERFACE_H

#include <stdbool.h>

typedef struct {
    int encoderPos;
    bool enterButtonWasPressed;
    bool backButtonWasPressed;
    bool knobButtonWasPressed;
} ui_StateType;

void ui_Init(void);

void ui_Process();

void ui_GetState(ui_StateType* state);

#endif // UI_INTERFACE_H