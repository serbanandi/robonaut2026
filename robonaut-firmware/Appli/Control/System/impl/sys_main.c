#include "sys_main.h"
#include "../sys_interface.h"
#include "stm32n6xx_nucleo.h"
#include "sys_init.h"

#include "LineSensor/LineSensor.h"
#include "MicroTimer/MicroTimer.h"

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

// static line_SplitInfoType lineSelectionFunc(int) {
//     static line_SplitInfoType lineSplitInfos[] = {
//         { LINE_SPLIT_RIGHT, 2 },
//         { LINE_SPLIT_RIGHT, 2 },
//         { LINE_SPLIT_STRAIGHT, 2 },
//         { LINE_SPLIT_RIGHT, 1 },
//         { LINE_SPLIT_STRAIGHT, 2 },
//         { LINE_SPLIT_RIGHT, 2 },
//         { LINE_SPLIT_STRAIGHT, 2 },
//         { LINE_SPLIT_RIGHT, 2 },
//         { LINE_SPLIT_STRAIGHT, 3 },
//         { LINE_SPLIT_RIGHT, 1 },
//         { LINE_SPLIT_STRAIGHT, 2 },
//         { LINE_SPLIT_STRAIGHT, 2 },
//         { LINE_SPLIT_RIGHT, 3 },
//         { LINE_SPLIT_LEFT, 3 }
//         // LINE_SPLIT_STRAIGHT,
//         // LINE_SPLIT_RIGHT,
//         // LINE_SPLIT_STRAIGHT,
//         // LINE_SPLIT_RIGHT,
//         // LINE_SPLIT_STRAIGHT,
//         // LINE_SPLIT_RIGHT,
//         // LINE_SPLIT_LEFT,
//         // LINE_SPLIT_RIGHT
//     };

//     if (lineSplitIndex >= sizeof(lineSplitInfos)/sizeof(lineSplitInfos[0])) {
//         lineSplitIndex = 0;
//     }
//     return lineSplitInfos[lineSplitIndex++];
// }
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

void sys_Run(void)
{
    static uint32_t encoderPos;
    static float encoderSpeed;
    static uint16_t totalProcessTimeUs;
    static bool targetReached;
    static uint32_t allblack_enc = 0;

    tel_RegisterR(&ui_RC_Trigger_Pulled, TEL_UINT8, "sys_RC_Trigger_Pulled", 100);
    tel_RegisterR(&encoderPos, TEL_UINT32, "sys_encoderPos", 100);
    tel_RegisterR(&encoderSpeed, TEL_FLOAT, "sys_encoderSpeed", 100);
    tel_RegisterR(&totalProcessTimeUs, TEL_UINT16, "sys_totalProcessTimeUs", 100);
    tel_RegisterR(&rc_currentRxState, TEL_UINT8, "rc_currentRxState", 200);
    tel_RegisterR(&latestPosition.fromNode, TEL_UINT8, "rc_fromNode", 200);
    tel_RegisterR(&latestPosition.toNode, TEL_UINT8, "rc_toNode", 200);
    tel_RegisterR(&latestPosition.nextNode, TEL_UINT8, "rc_nextNode", 200);
    tel_RegisterR(&latestPosition.positionPercent, TEL_UINT8, "rc_positionPercent", 200);
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

    // while (1)
    // {
    //     _sys_HandleParamTuning();
    //     encoderPos = drv_GetEncoderCount();
    //     encoderSpeed = drv_GetEncoderSpeed();

    //     test_ProcessAll();
    //     tel_Process();

    //     static uint32_t lastBlinkTime = 0;
    //     if (HAL_GetTick() - lastBlinkTime >= 500)
    //     {
    //         lastBlinkTime = HAL_GetTick();
    //         BSP_LED_Toggle(LED1);
    //     }
    // }

    while (1)
    {
        int32_t processingStartUs = MT_GetTick();
        _sys_HandleParamTuning();
        encoderPos = drv_GetEncoderCount();
        encoderSpeed = drv_GetEncoderSpeed();
        tel_Process();
        rc_Process();
        rc_currentRxState = rc_GetRxState();
        rc_GetPosition(&latestPosition);

        /* RC test code
        // print rc state and position data to ssd1306
        ssd1306_Fill(0);
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString("RC State:", Font_6x8, 0);
        char buf[20];
        sprintf(buf, "%d", rc_currentRxState);
        ssd1306_SetCursor(80, 0);
        ssd1306_WriteString(buf, Font_6x8, 0);
        ssd1306_SetCursor(0, 20);
        ssd1306_WriteString("From:", Font_6x8, 0);
        sprintf(buf, "%c", latestPosition.fromNode);
        ssd1306_SetCursor(50, 20);
        ssd1306_WriteString(buf, Font_6x8, 0);
        ssd1306_SetCursor(0, 30);
        ssd1306_WriteString("To:", Font_6x8, 0);
        sprintf(buf, "%c", latestPosition.toNode);
        ssd1306_SetCursor(50, 30);
        ssd1306_WriteString(buf, Font_6x8, 0);
        ssd1306_SetCursor(0, 40);
        ssd1306_WriteString("Next:", Font_6x8, 0);
        sprintf(buf, "%c", latestPosition.nextNode);
        ssd1306_SetCursor(50, 40);
        ssd1306_WriteString(buf, Font_6x8, 0);
        ssd1306_SetCursor(0, 50);
        ssd1306_WriteString("Progress:", Font_6x8, 0);
        sprintf(buf, "%d", latestPosition.positionPercent);
        ssd1306_SetCursor(60, 50);
        ssd1306_WriteString(buf, Font_6x8, 0);
        ssd1306_UpdateScreen();
        */

        static uint32_t lastBlinkTime = 0;
        if (HAL_GetTick() - lastBlinkTime >= 500)
        {
            lastBlinkTime = HAL_GetTick();
            BSP_LED_Toggle(LED1);
        }

        if (BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_SET)
        {
            MAGIC_ENABLED = false;
        }

        static int32_t lastProcessTime = 0;
        int32_t currentTime = MT_GetTick();
        int32_t diff = (currentTime - lastProcessTime + 0x10000) % 0x10000;
        if ((uint16_t) diff < controlPeriodUs)
            continue;
        lastProcessTime = currentTime;

        line_DetectionResultType lineResult = line_Process(lineSelectionFunc);
        if (doingYturn)
        {
            targetReached = fabs(encoderPos - encoderTarget) < 750;
            if (Ydir == 1 && targetReached)
            { // going back and target reached
                Ydir = -1;
                encoderTarget = encoderStart;
                tel_Log(TEL_LOG_DEBUG, "Enc: %u - Going forward", encoderPos);
            }
            else if (Ydir == -1 && targetReached)
            { // going forward and target reached
                Ydir = 0;
                doingYturn = false;
                tel_Log(TEL_LOG_DEBUG, "Enc: %u - Going back to line", encoderPos);
            }
            servo_SetAngle(SERVO_FRONT, Ydir);
            drv_SetSpeed(-Ydir * slowSpeed);
        }
        else if (MAGIC_ENABLED && !lineResult.allBlack)
        { // motor enabled and on line - follow
            allblack_enc = encoderPos;
            float control_signal = lc_Compute(-lineResult.detectedLinePos,
                                              P_GAIN, // P
                                              I_GAIN, // I
                                              D_GAIN, // D
                                              controlPeriodUs / 100000.0f);
            servo_SetAngle(SERVO_FRONT, control_signal);

            drv_SetSpeed(slowSpeed);
            drv_Enable(true);
        }
        else if (MAGIC_ENABLED && lineResult.allBlack)
        { // finished sequence
            if (allblack_enc + 3000 > encoderPos)
                continue;
            drv_Enable(false); // stop
            pathIndex++;
            lineSplitIndex = 0;

            encoderStart = encoderPos;
            encoderTarget = encoderPos - encoderDiff;

            doingYturn = true;
            Ydir = 1;
            tel_Log(TEL_LOG_DEBUG, "Enc: %u - Going backward", encoderPos);
            drv_Enable(true);
        }
        else
        {
            drv_Enable(false);
            drv_SetSpeed(0.0f);
            servo_SetAngle(SERVO_FRONT, 0.0f);
            lc_Init();
            lineSplitIndex = 0;
        }
        totalProcessTimeUs = (uint16_t) (((int32_t) MT_GetTick() - processingStartUs + 0x10000) % 0x10000);
    }
}
