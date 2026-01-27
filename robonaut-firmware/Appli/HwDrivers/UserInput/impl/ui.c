#include "ui.h"
#include <stdint.h>
#include "../ui_interface.h"
#include "IntHandler/int_interface.h"
#include "main.h"

static ui_StateType currentState = { 0 };
static int32_t encoderPosition = 0;

static bool enterButtonIsPressed = false;
static bool backButtonIsPressed = false;
static bool knobButtonIsPressed = false;
static uint32_t lastEnterButtonPressTime = 0;
static uint32_t lastBackButtonPressTime = 0;
static uint32_t lastKnobButtonPressTime = 0;

bool ui_RC_Trigger_Pulled;

void ui_Init(void)
{
    // register callback for PWM timer
    int_SubscribeToInt(INT_TIM_IC_CAPTURE, (int_CallbackFn) _ui_TimerCaptureCallback, UI_REMOTE_CONTROL_TIMER,
                       UI_REMOTE_CONTROL_TIMER);
    // start timer
    HAL_TIM_IC_Start_IT(UI_REMOTE_CONTROL_TIMER, TIM_CHANNEL_1);
}

void ui_Process()
{
    // Read encoder position
    static int lastA = 0;

    int currentA = HAL_GPIO_ReadPin(UI_A_GPIO_Port, UI_A_Pin);
    int currentB = HAL_GPIO_ReadPin(UI_B_GPIO_Port, UI_B_Pin);

    {
        static uint32_t lastEncoderDebounceTime = 0;
        const uint32_t ENCODER_DEBOUNCE_MS = 10;
        uint32_t now = HAL_GetTick();

        if (lastA == 0 && currentA == 1)
        {
            if ((now - lastEncoderDebounceTime) >= ENCODER_DEBOUNCE_MS)
            {
                if (currentB == 0)
                {
                    encoderPosition++;
                }
                else
                {
                    encoderPosition--;
                }
                lastEncoderDebounceTime = now;
            }
        }
    }
    // else if (lastA == 1 && currentA == 0)
    // {
    //     if (currentB == 0)
    //     {
    //         encoderPosition++;
    //     }
    //     else
    //     {
    //         encoderPosition--;
    //     }
    // }

    lastA = currentA;

    bool enterPressed = HAL_GPIO_ReadPin(UI_CONF_GPIO_Port, UI_CONF_Pin) == GPIO_PIN_RESET;
    bool backPressed = HAL_GPIO_ReadPin(UI_BACK_GPIO_Port, UI_BACK_Pin) == GPIO_PIN_RESET;
    bool knobPressed = HAL_GPIO_ReadPin(UI_PUSH_GPIO_Port, UI_PUSH_Pin) == GPIO_PIN_RESET;
    uint32_t currentTime = HAL_GetTick();
    if (enterPressed && !enterButtonIsPressed && (currentTime - lastEnterButtonPressTime) > 40)
    {
        currentState.enterButtonWasPressed = true;
        enterButtonIsPressed = true;
        lastEnterButtonPressTime = currentTime;
    }
    else if (!enterPressed && enterButtonIsPressed && (currentTime - lastEnterButtonPressTime) > 80)
    {
        enterButtonIsPressed = false;
        lastEnterButtonPressTime = currentTime;
    }
    if (backPressed && !backButtonIsPressed && (currentTime - lastBackButtonPressTime) > 40)
    {
        currentState.backButtonWasPressed = true;
        backButtonIsPressed = true;
        lastBackButtonPressTime = currentTime;
    }
    else if (!backPressed && backButtonIsPressed && (currentTime - lastBackButtonPressTime) > 80)
    {
        backButtonIsPressed = false;
        lastBackButtonPressTime = currentTime;
    }
    if (knobPressed && !knobButtonIsPressed && (currentTime - lastKnobButtonPressTime) > 40)
    {
        currentState.knobButtonWasPressed = true;
        knobButtonIsPressed = true;
        lastKnobButtonPressTime = currentTime;
    }
    else if (!knobPressed && knobButtonIsPressed && (currentTime - lastKnobButtonPressTime) > 80)
    {
        knobButtonIsPressed = false;
        lastKnobButtonPressTime = currentTime;
    }
}

void ui_GetButtonState(ui_StateType* state)
{
    if (state != NULL)
    {
        *state = currentState;
    }
    currentState.enterButtonWasPressed = false;
    currentState.backButtonWasPressed = false;
    currentState.knobButtonWasPressed = false;
}

void _ui_TimerCaptureCallback(void* timerHandle)
{
    TIM_HandleTypeDef* htim = (TIM_HandleTypeDef*) timerHandle;
    static uint32_t last_capture = 0;
    static uint32_t pulse_width = 0;
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        uint32_t current_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

        // Calculate duration between this edge and the last edge, handling overflow
        uint32_t diff;
        if (current_capture >= last_capture)
        {
            diff = current_capture - last_capture;
        }
        else
        {
            // Timer overflowed, add the timer period to account for wraparound
            diff = (htim->Init.Period - last_capture) + current_capture;
        }

        // Check Pin State to see what we just measured
        // If Pin is LOW now, we just finished the HIGH pulse (Falling Edge)
        if (HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_1) == GPIO_PIN_RESET)
        {
            pulse_width = diff; // This is your pulse width in timer ticks
            // Determine if trigger is pulled based on pulse width, pulled --> <1000us
            ui_RC_Trigger_Pulled = (pulse_width > 1800);
        }

        // Update last capture for next time
        last_capture = current_capture;
    }
}

int32_t ui_GetEncoderPosition()
{
    return encoderPosition;
}
