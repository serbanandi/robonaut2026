#include "sys_main.h"
#include "../sys_interface.h"
#include "stm32n6xx_nucleo.h"
#include "sys_init.h"

#include "LineSensor/ls_interface.h"
#include "MicroTimer/mt_interface.h"

#include "AnalogInputs/anlg_interface.h"
#include "Drive/drv_interface.h"
#include "HwTest/test_interface.h"
#include "LineController/lc_interface.h"
#include "LineProcessor/line_interface.h"
#include "RadioControl/rc_interface.h"
#include "SSD1306/ssd1306_fonts.h"
#include "SSD1306/ssd1306_interface.h"
#include "Servo/servo_interface.h"
#include "Telemetry/tel_interface.h"
#include "UserInput/ui_interface.h"

#include <stdarg.h>
#include <stdio.h>

bool MAGIC_ENABLED = false;
float slowSpeed = 0.1f;
bool doingYturn = false;
int8_t Ydir = 0;
float fastSpeed = 0.7f;
uint8_t lineSplitIndex = 0;
uint8_t pathIndex = 0;
float P_GAIN = 0.2f;
float I_GAIN = 0.005f;
float D_GAIN = 0.0f;
uint16_t controlPeriodUs = 5000; // 5 ms
rc_PositionType latestPosition;
rc_RxStateType rc_currentRxState;

extern bool ui_RC_Trigger_Pulled;

bool next = false;
uint32_t encoderStart = 0, encoderTarget = 0, encoderDiff = 95000;

static line_SplitDirectionType lineSelectionFunc(int)
{
    static uint8_t pathlens[] = { 3, 7, 7, 10, 10 };
    static line_SplitDirectionType path1[] = {
        LINE_SPLIT_LEFT,     // F
        LINE_SPLIT_LEFT,     // G
        LINE_SPLIT_STRAIGHT, // J
    };
    static line_SplitDirectionType path2[] = {
        LINE_SPLIT_RIGHT,    // F
        LINE_SPLIT_RIGHT,    // D
        LINE_SPLIT_RIGHT,    // C
        LINE_SPLIT_RIGHT,    // E
        LINE_SPLIT_LEFT,     // F
        LINE_SPLIT_LEFT,     // G
        LINE_SPLIT_STRAIGHT, // J
    };
    static line_SplitDirectionType path3[] = {
        LINE_SPLIT_LEFT,     // F
        LINE_SPLIT_RIGHT,    // G
        LINE_SPLIT_RIGHT,    // K
        LINE_SPLIT_RIGHT,    // N
        LINE_SPLIT_RIGHT,    // M
        LINE_SPLIT_LEFT,     // I
        LINE_SPLIT_STRAIGHT, // J
    };
    static line_SplitDirectionType path4[] = {
        LINE_SPLIT_LEFT,     // F
        LINE_SPLIT_LEFT,     // G
        LINE_SPLIT_RIGHT,    // J
        LINE_SPLIT_LEFT,     // L
        LINE_SPLIT_RIGHT,    // R
        LINE_SPLIT_RIGHT,    // T
        LINE_SPLIT_LEFT,     // Q
        LINE_SPLIT_RIGHT,    // M
        LINE_SPLIT_LEFT,     // I
        LINE_SPLIT_STRAIGHT, // J
    };
    static line_SplitDirectionType path5[] = {
        LINE_SPLIT_RIGHT,    // F
        LINE_SPLIT_LEFT,     // D
        LINE_SPLIT_RIGHT,    // A
        LINE_SPLIT_LEFT,     // B
        LINE_SPLIT_LEFT,     // E
        LINE_SPLIT_LEFT,     // H
        LINE_SPLIT_RIGHT,    // L
        LINE_SPLIT_RIGHT,    // O
        LINE_SPLIT_RIGHT,    // K
        LINE_SPLIT_STRAIGHT, // J
    };
    static line_SplitDirectionType* lineSplitDirections[] = { path1, path2, path3, path4, path5 };

    return lineSplitDirections[pathIndex][(lineSplitIndex++) % pathlens[pathIndex]];
}

static void _sys_displayStatus()
{
    static uint32_t lastStatusTime = 0;
    if (HAL_GetTick() - lastStatusTime >= 250)
    {
        anlg_BatteryStatusType batteryStatus;
        anlg_ReadBatteryStatus(&batteryStatus);

        lastStatusTime = HAL_GetTick();
        ssd1306_Fill(0);
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString("   Status", Font_11x18, 0);
        char buf[32];
        ssd1306_SetCursor(0, 20);
        snprintf(buf, sizeof(buf), "MotBat: %.2fV", batteryStatus.motorBatteryVoltage);
        ssd1306_WriteString(buf, Font_6x8, 0);
        ssd1306_SetCursor(0, 30);
        snprintf(buf, sizeof(buf), "AuxBat: %.2fV", batteryStatus.auxBatteryVoltage);
        ssd1306_WriteString(buf, Font_6x8, 0);
        ssd1306_UpdateScreen();
        BSP_LED_Toggle(LED1);
    }
}

void sys_Run(void)
{
    static uint16_t totalProcessTimeUs;
    static bool targetReached;
    static uint32_t allblack_enc = 0;

    tel_RegisterR(&totalProcessTimeUs, TEL_UINT16, "sys_totalProcessTimeUs", 100);
    tel_RegisterRW(&controlPeriodUs, TEL_UINT16, "sys_controlPeriodUs", 1000);
    tel_RegisterRW(&MAGIC_ENABLED, TEL_UINT8, "sys_ENABLE", 400);
    tel_RegisterRW(&slowSpeed, TEL_FLOAT, "sys_slowSpeed", 400);
    // tel_RegisterRW(&fastSpeed, TEL_FLOAT, "sys_fastSpeed", 400);
    tel_RegisterRW(&P_GAIN, TEL_FLOAT, "sys_P", 1000);
    tel_RegisterRW(&I_GAIN, TEL_FLOAT, "sys_I", 1000);
    tel_RegisterRW(&D_GAIN, TEL_FLOAT, "sys_D", 1000);
    tel_RegisterRW(&next, TEL_UINT8, "sys_next", 400);
    tel_RegisterR(&doingYturn, TEL_UINT8, "sys_doingYturn", 400);
    tel_RegisterRW(&pathIndex, TEL_UINT8, "sys_pathIndex", 400);
    tel_RegisterRW(&lineSplitIndex, TEL_UINT8, "sys_lineSplitIndex", 400);
    tel_RegisterRW(&encoderDiff, TEL_UINT32, "sys_encoderDiff", 400);

    tel_Log(TEL_LOG_INFO, "Entering main loop...");

    while (1)
    {
        _sys_HandleParamTuning();
        tel_Process();
        rc_Process();
        ui_Process();
        ls_Process();

        test_ProcessAll();

        //_sys_displayStatus();
        // bool newDataAvailable = false;
        // ls_Process(&newDataAvailable);

        // if (BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_SET)
        // {
        //     MAGIC_ENABLED = false;
        // }

        // static int32_t lastProcessTime = 0;
        // int32_t currentTime = mt_GetTick();
        // int32_t diff = (currentTime - lastProcessTime + 0x10000) % 0x10000;
        // if ((uint16_t) diff < controlPeriodUs)
        //     continue;
        // lastProcessTime = currentTime;

        // line_DetectionResultType lineResult = line_Process(lineSelectionFunc);
        // if (doingYturn)
        // {
        //     targetReached = fabs(encoderPos - encoderTarget) < 750;
        //     if (Ydir == 1 && targetReached)
        //     { // going back and target reached
        //         Ydir = -1;
        //         encoderTarget = encoderStart;
        //         tel_Log(TEL_LOG_DEBUG, "Enc: %u - Going forward", encoderPos);
        //     }
        //     else if (Ydir == -1 && targetReached)
        //     { // going forward and target reached
        //         Ydir = 0;
        //         doingYturn = false;
        //         tel_Log(TEL_LOG_DEBUG, "Enc: %u - Going back to line", encoderPos);
        //     }
        //     servo_SetAngle(SERVO_FRONT, Ydir);
        //     drv_SetSpeed(-Ydir * slowSpeed);
        // }
        // else if (MAGIC_ENABLED && !lineResult.allBlack)
        // { // motor enabled and on line - follow
        //     allblack_enc = encoderPos;
        //     float control_signal = lc_Compute(-lineResult.detectedLinePos,
        //                                       P_GAIN, // P
        //                                       I_GAIN, // I
        //                                       D_GAIN, // D
        //                                       controlPeriodUs / 100000.0f);
        //     servo_SetAngle(SERVO_FRONT, control_signal);

        //     drv_SetSpeed(slowSpeed);
        //     drv_Enable(true);
        // }
        // else if (MAGIC_ENABLED && lineResult.allBlack)
        // { // finished sequence
        //     if (allblack_enc + 3000 > encoderPos)
        //         continue;
        //     drv_Enable(false); // stop
        //     pathIndex++;
        //     lineSplitIndex = 0;

        //     encoderStart = encoderPos;
        //     encoderTarget = encoderPos - encoderDiff;

        //     doingYturn = true;
        //     Ydir = 1;
        //     tel_Log(TEL_LOG_DEBUG, "Enc: %u - Going backward", encoderPos);
        //     drv_Enable(true);
        // }
        // else
        // {
        //     drv_Enable(false);
        //     drv_SetSpeed(0.0f);
        //     servo_SetAngle(SERVO_FRONT, 0.0f);
        //     lc_Init();
        //     lineSplitIndex = 0;
        // }
        // totalProcessTimeUs = (uint16_t) (((int32_t) mt_GetTick() - processingStartUs + 0x10000) % 0x10000);
    }
}
