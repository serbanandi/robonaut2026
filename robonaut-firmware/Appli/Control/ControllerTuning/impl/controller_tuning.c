#include "../tuning_interface.h"
#include "controller_tuning.h"

#include "SimpleLogger/SimpleLogger.h"
#include "LineProcessor/line_interface.h"

#include "SSD1306/ssd1306_interface.h"
#include "SSD1306/ssd1306_fonts.h"
#include "UserInput/ui_interface.h"

#include <stdio.h>

//extern line_DetectionResultType line_detection_result;
//
//static float tuning_parameters[6];
//static tuning_ParameterType param;
//static tuning_StateType state;
//static tuning_ParametersType* applied_params;
//
//
//static void _tuning_PrintUI(void) {
//    char buffer[64];
//    ssd1306_Fill(0);
//
//    ssd1306_SetCursor(0, 0);
//    switch(state) {
//        case IDLE:
//            snprintf(buffer, sizeof(buffer), "IDLE\n");
//            break;
//        case SELECT_PARAM:
//            snprintf(buffer, sizeof(buffer), "SELECT_PARAM\n");
//            break;
//        case ADJUST_PARAM:
//            snprintf(buffer, sizeof(buffer), "ADJUST_PARAM\n");
//            break;
//        case APPLY_PARAM:
//            snprintf(buffer, sizeof(buffer), "APPLY_PARAM\n");
//            break;
//        default:
//            snprintf(buffer, sizeof(buffer), "State: UNKNOWN\n");
//            break;
//    }
//    ssd1306_WriteString(buffer, Font_6x8, 0);
//if(state != APPLY_PARAM){
//    ssd1306_SetCursor(0, 10);
//    snprintf(buffer, sizeof(buffer), "Threshold: %d\n", (int) tuning_parameters[THRESHOLD]);
//	ssd1306_WriteString(buffer, Font_6x8, ((state == SELECT_PARAM || state == ADJUST_PARAM)  && param == THRESHOLD));
//
//    ssd1306_SetCursor(0, 20);
//    snprintf(buffer, sizeof(buffer), "P: %.3f\n", tuning_parameters[P_COEFF]);
//    ssd1306_WriteString(buffer, Font_6x8, ((state == SELECT_PARAM || state == ADJUST_PARAM)  && param == P_COEFF));
//
//    ssd1306_SetCursor(0, 30);
//    snprintf(buffer, sizeof(buffer), "I: %.3f\n", tuning_parameters[I_COEFF]);
//    ssd1306_WriteString(buffer, Font_6x8, ((state == SELECT_PARAM || state == ADJUST_PARAM)  && param == I_COEFF));
//
//    ssd1306_SetCursor(0, 40);
//    snprintf(buffer, sizeof(buffer), "D: %.3f\n", tuning_parameters[D_COEFF]);
//    ssd1306_WriteString(buffer, Font_6x8, ((state == SELECT_PARAM || state == ADJUST_PARAM)  && param == D_COEFF));
//
//    ssd1306_SetCursor(0, 50);
//    snprintf(buffer, sizeof(buffer), "Speed: %.2f\n", tuning_parameters[SPEED]);
//
//    ssd1306_WriteString(buffer, Font_6x8, ((state == SELECT_PARAM || state == ADJUST_PARAM)  && param == SPEED));
//
//    ssd1306_SetCursor(0, 60);
//    snprintf(buffer, sizeof(buffer), "Mode: %d\n", (int) tuning_parameters[MODE]);
//    ssd1306_WriteString(buffer, Font_6x8, ((state == SELECT_PARAM || state == ADJUST_PARAM)  && param == MODE));
//}
//else{
//	ssd1306_SetCursor(0, 10);
//	snprintf(buffer, sizeof(buffer), "p: %f\n", line_detection_result.position);
//	ssd1306_WriteString(buffer, Font_6x8, 0);
//
//    ssd1306_SetCursor(0, 20);
//    switch (line_detection_result.lineType) {
//        case LINE_NO_LINE:
//            snprintf(buffer, sizeof(buffer), "Line: NO LINE\n");
//            break;
//        case LINE_SINGLE_LINE:
//            snprintf(buffer, sizeof(buffer), "Line: SINGLE LINE\n");
//            break;
//        case LINE_TRIPLE_LINE:
//            snprintf(buffer, sizeof(buffer), "Line: TRIPLE LINE\n");
//            break;
//        case LINE_TRIPLE_LINE_DASHED:
//            snprintf(buffer, sizeof(buffer), "Line: TRIPLE DASHED\n");
//            break;
//        default:
//            snprintf(buffer, sizeof(buffer), "Line: UNKNOWN\n");
//            break;
//    }
//    ssd1306_WriteString(buffer, Font_6x8, 0);
//}
//    ssd1306_UpdateScreen();
//}
//
//static void _tuning_AdjustParameter(tuning_ParameterType parameter, int32_t adjustment) {
//    float adjustmentFactor = 0.0f;
//
//    switch (parameter) {
//        case THRESHOLD:
//            adjustmentFactor = 1.0f;
//            break;
//        case I_COEFF:
//        case P_COEFF:
//            adjustmentFactor = 0.001f;
//            break;
//        case D_COEFF:
//            adjustmentFactor = 0.01f;
//            break;
//        case SPEED:
//            adjustmentFactor = 0.01f;
//            break;
//        case MODE:
//            adjustmentFactor = 1.0f;
//            break;
//        default:
//            return; // Invalid parameter
//    }
//
//    tuning_parameters[parameter] += adjustment * adjustmentFactor;
//
//    // Ensure parameters stay within valid ranges
//    if (tuning_parameters[THRESHOLD] < 0.0f) tuning_parameters[THRESHOLD] = 0.0f;
//    if (tuning_parameters[P_COEFF] < 0.0f) tuning_parameters[P_COEFF] = 0.0f;
//    if (tuning_parameters[I_COEFF] < 0.0f) tuning_parameters[I_COEFF] = 0.0f;
//    if (tuning_parameters[D_COEFF] < 0.0f) tuning_parameters[D_COEFF] = 0.0f;
//    if (tuning_parameters[SPEED] < -1.0f) tuning_parameters[SPEED] = -1.0f;
//    if (tuning_parameters[SPEED] > 1.0f) tuning_parameters[SPEED] = 1.0f;
//    if (tuning_parameters[MODE] < 0.0f) tuning_parameters[MODE] = 0.0f;
//    if (tuning_parameters[MODE] > 1.0f) tuning_parameters[MODE] = 1.0f;
//}
//
//static void _tuning_ApplyParameters(void) {
//    applied_params->threshold = (uint16_t)tuning_parameters[THRESHOLD];
//    applied_params->p_coeff = tuning_parameters[P_COEFF];
//    applied_params->i_coeff = tuning_parameters[I_COEFF];
//    applied_params->d_coeff = tuning_parameters[D_COEFF];
//    applied_params->speed = tuning_parameters[SPEED];
//    applied_params->mode = (tuning_parameters[MODE] >= 0.5f) ? true : false;
//}
//
//void tuning_Init(tuning_ParametersType* params){
//    // Initialize controller tuning here
//    applied_params = params;
//
//    tuning_parameters[THRESHOLD] = 900.0f;
//    tuning_parameters[P_COEFF] = 0.105f;
//    tuning_parameters[I_COEFF] = 0.005f;
//    tuning_parameters[D_COEFF] = 0.32f;
//    tuning_parameters[SPEED] = 0.22f;
//    tuning_parameters[MODE] = 0.0f;
//
//    _tuning_ApplyParameters();
//
//    param = THRESHOLD;
//    state = IDLE;
//}
//
//void tuning_Process(void){
//    static int32_t lastEncoderPos = 0;
//    static int32_t currentEncoderPos = 0;
//    ui_StateType ui_state;
//
//    ui_GetButtonState(&ui_state);
//    lastEncoderPos = currentEncoderPos;
//    currentEncoderPos = ui_GetEncoderPosition();
//
//    _tuning_PrintUI();
//
//    // Process controller tuning here
//    switch (state) {
//        case IDLE:          // Stop motor
//            applied_params->motor_enabled = false;
//
//            state = SELECT_PARAM;
//            break;
//        case SELECT_PARAM:  // Handle parameter selection
//            if (ui_state.knobButtonWasPressed) {state = ADJUST_PARAM;}
//            else if (ui_state.enterButtonWasPressed) {state = APPLY_PARAM;}
//            else if (currentEncoderPos != lastEncoderPos) {
//                param = (tuning_ParameterType)(param + (currentEncoderPos - lastEncoderPos)) % 6;
//            }
//            break;
//        case ADJUST_PARAM:  // Handle parameter adjustment
//            if (ui_state.knobButtonWasPressed) {state = SELECT_PARAM;}
//            else if (ui_state.enterButtonWasPressed) {state = APPLY_PARAM;}
//            else if (currentEncoderPos != lastEncoderPos) { _tuning_AdjustParameter(param, currentEncoderPos - lastEncoderPos);}
//            else if (ui_state.backButtonWasPressed) {state = SELECT_PARAM;}
//            break;
//        case APPLY_PARAM:   // Apply the adjusted parameter
//            if (ui_state.backButtonWasPressed) {state = IDLE; applied_params->motor_enabled = false; break;}
//
//            _tuning_ApplyParameters();
//            applied_params->motor_enabled = true;
//            break;
//        default:
//            break;
//    }
//
//
//}
//
//void tuning_Stop(void)
//{
//    state = IDLE;
//}
