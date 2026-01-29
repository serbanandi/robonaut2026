#include "ui.h"
#include <stdint.h>
#include "../ui_interface.h"
#include "IntHandler/int_interface.h"
#include "main.h"

static ui_StateType _ui_currentState = { 0 };
static int32_t _ui_encoderPosition = 0;

static bool _ui_enterButtonIsPressed = false;
static bool _ui_backButtonIsPressed = false;
static bool _ui_knobButtonIsPressed = false;
static uint32_t _ui_lastEnterButtonPressTime = 0;
static uint32_t _ui_lastBackButtonPressTime = 0;
static uint32_t _ui_lastKnobButtonPressTime = 0;

static volatile bool _ui_RC_Trigger_Pulled = false;

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
                    _ui_encoderPosition++;
                }
                else
                {
                    _ui_encoderPosition--;
                }
                lastEncoderDebounceTime = now;
            }
        }
    }
    // else if (lastA == 1 && currentA == 0)
    // {
    //     if (currentB == 0)
    //     {
    //         _ui_encoderPosition++;
    //     }
    //     else
    //     {
    //         _ui_encoderPosition--;
    //     }
    // }

    lastA = currentA;

    bool enterPressed = HAL_GPIO_ReadPin(UI_CONF_GPIO_Port, UI_CONF_Pin) == GPIO_PIN_RESET;
    bool backPressed = HAL_GPIO_ReadPin(UI_BACK_GPIO_Port, UI_BACK_Pin) == GPIO_PIN_RESET;
    bool knobPressed = HAL_GPIO_ReadPin(UI_PUSH_GPIO_Port, UI_PUSH_Pin) == GPIO_PIN_RESET;
    uint32_t currentTime = HAL_GetTick();
    if (enterPressed && !_ui_enterButtonIsPressed && (currentTime - _ui_lastEnterButtonPressTime) > 40)
    {
        _ui_currentState.enterButtonWasPressed = true;
        _ui_enterButtonIsPressed = true;
        _ui_lastEnterButtonPressTime = currentTime;
    }
    else if (!enterPressed && _ui_enterButtonIsPressed && (currentTime - _ui_lastEnterButtonPressTime) > 80)
    {
        _ui_enterButtonIsPressed = false;
        _ui_lastEnterButtonPressTime = currentTime;
    }
    if (backPressed && !_ui_backButtonIsPressed && (currentTime - _ui_lastBackButtonPressTime) > 40)
    {
        _ui_currentState.backButtonWasPressed = true;
        _ui_backButtonIsPressed = true;
        _ui_lastBackButtonPressTime = currentTime;
    }
    else if (!backPressed && _ui_backButtonIsPressed && (currentTime - _ui_lastBackButtonPressTime) > 80)
    {
        _ui_backButtonIsPressed = false;
        _ui_lastBackButtonPressTime = currentTime;
    }
    if (knobPressed && !_ui_knobButtonIsPressed && (currentTime - _ui_lastKnobButtonPressTime) > 40)
    {
        _ui_currentState.knobButtonWasPressed = true;
        _ui_knobButtonIsPressed = true;
        _ui_lastKnobButtonPressTime = currentTime;
    }
    else if (!knobPressed && _ui_knobButtonIsPressed && (currentTime - _ui_lastKnobButtonPressTime) > 80)
    {
        _ui_knobButtonIsPressed = false;
        _ui_lastKnobButtonPressTime = currentTime;
    }
}

void ui_GetButtonState(ui_StateType* state)
{
    if (state != NULL)
    {
        *state = _ui_currentState;
    }
    _ui_currentState.enterButtonWasPressed = false;
    _ui_currentState.backButtonWasPressed = false;
    _ui_currentState.knobButtonWasPressed = false;
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
            _ui_RC_Trigger_Pulled = (pulse_width > 1800);
        }

        // Update last capture for next time
        last_capture = current_capture;
    }
}

int32_t ui_GetEncoderPosition()
{
    return _ui_encoderPosition;
}

bool ui_GetRCTriggerState()
{
    return _ui_RC_Trigger_Pulled;
}