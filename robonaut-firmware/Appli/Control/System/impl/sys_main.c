#include "../sys_interface.h"
#include "sys_main.h"
#include "sys_init.h"
#include "stm32n6xx_nucleo.h"

#include "LineSensor/LineSensor.h"
#include "MicroTimer/MicroTimer.h"

#include "SSD1306/ssd1306_interface.h"
#include "SSD1306/ssd1306_fonts.h"
#include "UserInput/ui_interface.h"
#include "Servo/servo_interface.h"
#include "Drive/drv_interface.h"
#include "HwTest/test_interface.h"
#include "LineProcessor/line_interface.h"
#include "Telemetry/tel_interface.h"
#include "LineController/lc_interface.h"

#include <stdio.h>
#include <stdarg.h>

bool MAGIC_ENABLED = false;
float slowSpeed = 0.1f;
float fastSpeed = 0.7f;
uint8_t lineSplitIndex = 0;
float P_GAIN = 0.2f;
float I_GAIN = 0.005f;
float D_GAIN = 0.0f;
uint16_t controlPeriodUs = 5000; // 5 ms
bool testModeEnabled = false;
float testServoAngle = 0.0f;

static line_SplitInfoType lineSelectionFunc(int) {
    static line_SplitInfoType lineSplitInfos[] = {
        { LINE_SPLIT_RIGHT, 2 },
        { LINE_SPLIT_RIGHT, 2 },
        { LINE_SPLIT_STRAIGHT, 2 },
        { LINE_SPLIT_RIGHT, 1 },
        { LINE_SPLIT_STRAIGHT, 2 },
        { LINE_SPLIT_RIGHT, 2 },
        { LINE_SPLIT_STRAIGHT, 2 },
        { LINE_SPLIT_RIGHT, 2 },
        { LINE_SPLIT_STRAIGHT, 3 },
        { LINE_SPLIT_RIGHT, 1 },
        { LINE_SPLIT_STRAIGHT, 2 },
        { LINE_SPLIT_STRAIGHT, 2 },
        { LINE_SPLIT_RIGHT, 3 },
        { LINE_SPLIT_LEFT, 3 }
        // LINE_SPLIT_STRAIGHT, 
        // LINE_SPLIT_RIGHT, 
        // LINE_SPLIT_STRAIGHT, 
        // LINE_SPLIT_RIGHT, 
        // LINE_SPLIT_STRAIGHT, 
        // LINE_SPLIT_RIGHT, 
        // LINE_SPLIT_LEFT, 
        // LINE_SPLIT_RIGHT
    };

    if (lineSplitIndex >= sizeof(lineSplitInfos)/sizeof(lineSplitInfos[0])) {
        lineSplitIndex = 0;
    }
    return lineSplitInfos[lineSplitIndex++];
}

void sys_Run(void)
{
    static uint32_t encoderPos;
    static float encoderSpeed;
    static uint16_t totalProcessTimeUs;

    tel_RegisterR(&encoderPos, TEL_UINT32, "sys_encoderPos", 100);
    tel_RegisterR(&encoderSpeed, TEL_FLOAT, "sys_encoderSpeed", 100);
    tel_RegisterR(&totalProcessTimeUs, TEL_UINT16, "sys_totalProcessTimeUs", 100);
    tel_RegisterRW(&controlPeriodUs, TEL_UINT16, "sys_controlPeriodUs", 1000);
    tel_RegisterRW(&MAGIC_ENABLED, TEL_UINT8, "sys_ENABLE", 400);
    tel_RegisterRW(&slowSpeed, TEL_FLOAT, "sys_slowSpeed", 400);
    //tel_RegisterRW(&fastSpeed, TEL_FLOAT, "sys_fastSpeed", 400);
    tel_RegisterRW(&P_GAIN, TEL_FLOAT, "sys_P", 1000);
    tel_RegisterRW(&I_GAIN, TEL_FLOAT, "sys_I", 1000);
    tel_RegisterRW(&D_GAIN, TEL_FLOAT, "sys_D", 1000);
    tel_RegisterRW(&testModeEnabled, TEL_UINT8, "sys_testModeEnabled", 1000);
    tel_RegisterRW(&testServoAngle, TEL_FLOAT, "sys_testServoAngle", 1000);


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
        if ((uint16_t)diff < controlPeriodUs)
            continue;
        lastProcessTime = currentTime;
        
        line_DetectionResultType lineResult = line_Process(lineSelectionFunc);
        if (testModeEnabled) {
            servo_SetAngle(SERVO_FRONT, testServoAngle);
            drv_SetSpeed(0.0f);
            drv_Enable(false);
        }
        else if(MAGIC_ENABLED) {
            float control_signal = lc_Compute(-lineResult.detectedLinePos,
                                                P_GAIN, //P 
                                                I_GAIN, //I
                                                D_GAIN,  //D
												controlPeriodUs / 100000.0f);
            servo_SetAngle(SERVO_FRONT, control_signal);
            
            drv_SetSpeed(slowSpeed);
            drv_Enable(true);
        } else {
            // drv_Enable(false);
            drv_SetSpeed(0.0f);
            servo_SetAngle(SERVO_FRONT, 0.0f);
            lc_Init();
            lineSplitIndex = 0;
        }
        totalProcessTimeUs = (uint16_t)(((int32_t)MT_GetTick() - processingStartUs + 0x10000) % 0x10000);
    }
}

