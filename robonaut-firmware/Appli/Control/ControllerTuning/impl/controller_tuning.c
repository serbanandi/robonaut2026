#include "../tuning_interface.h"
#include "controller_tuning.h"

#include "SimpleLogger/SimpleLogger.h"

#include "SSD1306/ssd1306_interface.h"
#include "SSD1306/ssd1306_fonts.h"
#include "UserInput/ui_interface.h"

static float tuning_parameters[6];
static tuning_ParameterType param;
static tuning_StateType state;
static tuning_ParametersType* applied_params;


static void _tuning_PrintUI(void) {
    char buffer[64];
    ssd1306_Fill(0);

    ssd1306_SetCursor(0, 0);
    switch(state) {
        case IDLE:
            snprintf(buffer, sizeof(buffer), "IDLE\n");
            break;
        case SELECT_PARAM:
            snprintf(buffer, sizeof(buffer), "SELECT_PARAM\n");
            break;
        case ADJUST_PARAM:
            snprintf(buffer, sizeof(buffer), "ADJUST_PARAM\n");
            break;
        case APPLY_PARAM:
            snprintf(buffer, sizeof(buffer), "APPLY_PARAM\n");
            break;
        default:
            snprintf(buffer, sizeof(buffer), "State: UNKNOWN\n");
            break;
    }
    ssd1306_WriteString(buffer, Font_6x8, 0);

    ssd1306_SetCursor(0, 10);
    snprintf(buffer, sizeof(buffer), "Threshold: %d\n", (int) tuning_parameters[THRESHOLD]);
    if ((state == SELECT_PARAM || state == ADJUST_PARAM)  && param == THRESHOLD)
        ssd1306_WriteString(buffer, Font_7x10, 0);
    else
        ssd1306_WriteString(buffer, Font_6x8, 0);

    ssd1306_SetCursor(0, 20);
    snprintf(buffer, sizeof(buffer), "P: %d.%d%d\n", (int) tuning_parameters[P_COEFF], 
                                                     (int)((tuning_parameters[P_COEFF] - (int)tuning_parameters[P_COEFF]) * 10),
                                                     (int)((tuning_parameters[P_COEFF] - (int)tuning_parameters[P_COEFF]) * 100) % 10);
    if ((state == SELECT_PARAM || state == ADJUST_PARAM)  && param == P_COEFF)
        ssd1306_WriteString(buffer, Font_7x10, 1);
    else
        ssd1306_WriteString(buffer, Font_6x8, 0);

    ssd1306_SetCursor(0, 30);
    snprintf(buffer, sizeof(buffer), "I: %d.%d%d\n", (int) tuning_parameters[I_COEFF], 
                                                     (int)((tuning_parameters[I_COEFF] - (int)tuning_parameters[I_COEFF]) * 10),
                                                     (int)((tuning_parameters[I_COEFF] - (int)tuning_parameters[I_COEFF]) * 100) % 10);
    if ((state == SELECT_PARAM || state == ADJUST_PARAM)  && param == I_COEFF)
        ssd1306_WriteString(buffer, Font_7x10, 0);
    else
        ssd1306_WriteString(buffer, Font_6x8, 0);

    ssd1306_SetCursor(0, 40);
    snprintf(buffer, sizeof(buffer), "D: %d.%d%d\n", (int) tuning_parameters[D_COEFF], 
                                                     (int)((tuning_parameters[D_COEFF] - (int)tuning_parameters[D_COEFF]) * 10),
                                                     (int)((tuning_parameters[D_COEFF] - (int)tuning_parameters[D_COEFF]) * 100) % 10);
    if ((state == SELECT_PARAM || state == ADJUST_PARAM)  && param == D_COEFF)
        ssd1306_WriteString(buffer, Font_7x10, 0);
    else
        ssd1306_WriteString(buffer, Font_6x8, 0);

    ssd1306_SetCursor(0, 50);
    snprintf(buffer, sizeof(buffer), "Speed: %d.%d%d\n", (int) tuning_parameters[SPEED], 
                                                         (int)((tuning_parameters[SPEED] - (int)tuning_parameters[SPEED]) * 10),
                                                         (int)((tuning_parameters[SPEED] - (int)tuning_parameters[SPEED]) * 100) % 10);
    if ((state == SELECT_PARAM || state == ADJUST_PARAM)  && param == SPEED)
        ssd1306_WriteString(buffer, Font_7x10, 0);
    else
        ssd1306_WriteString(buffer, Font_6x8, 0);

    ssd1306_SetCursor(0, 60);
    snprintf(buffer, sizeof(buffer), "Mode: %d\n", (int) tuning_parameters[MODE]);
    if ((state == SELECT_PARAM || state == ADJUST_PARAM)  && param == MODE)
        ssd1306_WriteString(buffer, Font_7x10, 0);
    else
        ssd1306_WriteString(buffer, Font_6x8, 0);

    ssd1306_UpdateScreen();
}

static void _tuning_AdjustParameter(tuning_ParameterType parameter, int32_t adjustment) {
    float adjustmentFactor = 0.0f;

    switch (parameter) {
        case THRESHOLD:
            adjustmentFactor = 1.0f;
            break;
        case P_COEFF:
        case I_COEFF:
        case D_COEFF:
            adjustmentFactor = 0.001f;
            break;
        case SPEED:
            adjustmentFactor = 0.01f;
            break;
        case MODE:
            adjustmentFactor = 1.0f;
            break;
        default:
            return; // Invalid parameter
    }

    tuning_parameters[parameter] += adjustment * adjustmentFactor;

    // Ensure parameters stay within valid ranges
    if (tuning_parameters[THRESHOLD] < 0.0f) tuning_parameters[THRESHOLD] = 0.0f;
    if (tuning_parameters[P_COEFF] < 0.0f) tuning_parameters[P_COEFF] = 0.0f;
    if (tuning_parameters[I_COEFF] < 0.0f) tuning_parameters[I_COEFF] = 0.0f;
    if (tuning_parameters[D_COEFF] < 0.0f) tuning_parameters[D_COEFF] = 0.0f;
    if (tuning_parameters[SPEED] < -1.0f) tuning_parameters[SPEED] = -1.0f;
    if (tuning_parameters[SPEED] > 1.0f) tuning_parameters[SPEED] = 1.0f;
    if (tuning_parameters[MODE] < 0.0f) tuning_parameters[MODE] = 0.0f;
    if (tuning_parameters[MODE] > 1.0f) tuning_parameters[MODE] = 1.0f;
}

static void _tuning_ApplyParameters(void) {
    applied_params->threshold = (uint16_t)tuning_parameters[THRESHOLD];
    applied_params->p_coeff = tuning_parameters[P_COEFF];
    applied_params->i_coeff = tuning_parameters[I_COEFF];
    applied_params->d_coeff = tuning_parameters[D_COEFF];
    applied_params->speed = tuning_parameters[SPEED];
    applied_params->mode = (tuning_parameters[MODE] >= 0.5f) ? true : false;
}

void tuning_Init(const tuning_ParametersType* params){
    // Initialize controller tuning here
    applied_params = params;

    tuning_parameters[THRESHOLD] = 1600.0f;
    tuning_parameters[P_COEFF] = 1.0f;
    tuning_parameters[I_COEFF] = 0.0f;
    tuning_parameters[D_COEFF] = 0.0f;
    tuning_parameters[SPEED] = 0.0f;
    tuning_parameters[MODE] = 0.;

    _tuning_ApplyParameters();

    param = THRESHOLD;
    state = IDLE;
}

void tuning_Process(void){
    static int32_t lastEncoderPos = 0;
    static int32_t currentEncoderPos = 0;
    float servo_setpoint = 0.0f;
    ui_StateType ui_state;

    ui_Process();
    ui_GetButtonState(&ui_state);
    lastEncoderPos = currentEncoderPos;
    currentEncoderPos = ui_GetEncoderPosition();

    _tuning_PrintUI();

    // Process controller tuning here
    switch (state) {
        case IDLE:          // Stop motor
            applied_params->motor_enabled = false;

            state = SELECT_PARAM;
            break;
        case SELECT_PARAM:  // Handle parameter selection
            if (ui_state.knobButtonWasPressed) {state = ADJUST_PARAM;}
            else if (ui_state.enterButtonWasPressed) {state = APPLY_PARAM;}
            else if (currentEncoderPos != lastEncoderPos) {
                param = (tuning_ParameterType)(param + (currentEncoderPos - lastEncoderPos)) % 6;
            }
            break;
        case ADJUST_PARAM:  // Handle parameter adjustment
            if (ui_state.knobButtonWasPressed) {state = SELECT_PARAM;}
            else if (ui_state.enterButtonWasPressed) {state = APPLY_PARAM;}
            else if (currentEncoderPos != lastEncoderPos) { _tuning_AdjustParameter(param, currentEncoderPos - lastEncoderPos);}
            else if (ui_state.backButtonWasPressed) {state = SELECT_PARAM;}
            break;
        case APPLY_PARAM:   // Apply the adjusted parameter
            if (ui_state.backButtonWasPressed) {state = IDLE; applied_params->motor_enabled = false; break;}
            
            _tuning_ApplyParameters();
            applied_params->motor_enabled = true;
            break;
        default:
            break;
    }

    
}
