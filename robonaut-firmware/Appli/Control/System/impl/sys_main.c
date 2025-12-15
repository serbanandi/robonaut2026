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
bool doingYturn = false;
int8_t Ydir = 0;
float slowSpeed = 0.2f;
float fastSpeed = 0.7f;
uint8_t lineSplitIndex = 0;
uint8_t pathIndex = 0;
float P_GAIN = 0.2f;
float I_GAIN = 0.005f;
float D_GAIN = 0.0f;
bool next = false;
uint32_t encoderStart = 0, encoderTarget = 0, encoderDiff=95000;

static line_SplitDirectionType lineSelectionFunc(int) {
    static line_SplitDirectionType path1[] = {
			LINE_SPLIT_STRAIGHT,	//F
			LINE_SPLIT_STRAIGHT,	//G
			LINE_SPLIT_STRAIGHT,	//J
	};
    static line_SplitDirectionType path2[] = {
			LINE_SPLIT_RIGHT,		//F
			LINE_SPLIT_RIGHT,		//D
			LINE_SPLIT_RIGHT,		//C
			LINE_SPLIT_RIGHT,		//E
			LINE_SPLIT_STRAIGHT,	//F
			LINE_SPLIT_STRAIGHT,	//G
			LINE_SPLIT_STRAIGHT,	//J
	};
    static line_SplitDirectionType path3[] = {
			LINE_SPLIT_STRAIGHT,	//F
			LINE_SPLIT_RIGHT,		//G
			LINE_SPLIT_RIGHT,		//K
			LINE_SPLIT_RIGHT,		//N
			LINE_SPLIT_RIGHT,		//M
			LINE_SPLIT_STRAIGHT,	//I
			LINE_SPLIT_STRAIGHT,	//J
	};
    static line_SplitDirectionType path4[] = {
			LINE_SPLIT_STRAIGHT,	//F
			LINE_SPLIT_STRAIGHT,	//G
			LINE_SPLIT_RIGHT,		//J
			LINE_SPLIT_STRAIGHT,	//L
			LINE_SPLIT_RIGHT,		//R
			LINE_SPLIT_RIGHT,		//T
			LINE_SPLIT_STRAIGHT,	//Q
			LINE_SPLIT_RIGHT,		//M
			LINE_SPLIT_STRAIGHT,	//I
			LINE_SPLIT_STRAIGHT,	//J
	};
    static line_SplitDirectionType path5[] = {
			LINE_SPLIT_RIGHT,		//F
			LINE_SPLIT_STRAIGHT,	//D
			LINE_SPLIT_RIGHT,		//A
			LINE_SPLIT_STRAIGHT,	//B
			LINE_SPLIT_STRAIGHT,	//E
			LINE_SPLIT_STRAIGHT,	//H
			LINE_SPLIT_RIGHT,		//L
			LINE_SPLIT_RIGHT,		//O
			LINE_SPLIT_RIGHT,		//K
			LINE_SPLIT_STRAIGHT,	//J
	};
	static line_SplitDirectionType* lineSplitDirections[] = {path1, path2, path3, path4, path5};

    return lineSplitDirections[pathIndex][lineSplitIndex++];
}

void sys_Run(void)
{
    static uint32_t encoderPos;
    static float encoderSpeed;
    static bool targetReached;

    tel_RegisterR(&encoderPos, TEL_UINT32, "sys_encoderPos", 100);
    tel_RegisterR(&encoderSpeed, TEL_FLOAT, "sys_encoderSpeed", 100);
    tel_RegisterRW(&MAGIC_ENABLED, TEL_UINT8, "sys_ENABLE", 400);
    tel_RegisterRW(&slowSpeed, TEL_FLOAT, "sys_slowSpeed", 400);
    tel_RegisterRW(&fastSpeed, TEL_FLOAT, "sys_fastSpeed", 400);
    tel_RegisterRW(&P_GAIN, TEL_FLOAT, "sys_P", 400);
    tel_RegisterRW(&I_GAIN, TEL_FLOAT, "sys_I", 400);
    tel_RegisterRW(&D_GAIN, TEL_FLOAT, "sys_D", 400);
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

        static uint32_t lastProcessTime = 0;
        uint32_t currentTime = HAL_GetTick();
        if (currentTime - lastProcessTime < 10)
            continue;
        lastProcessTime = currentTime;
        line_DetectionResultType lineResult = line_Process(lineSelectionFunc);
        	if (doingYturn) {
        		targetReached = fabs(encoderPos - encoderTarget) < 750;
        		if (Ydir==1 && targetReached){ // going back and target reached
        			Ydir = -1;
        			encoderTarget = encoderStart;
        			tel_Log(TEL_LOG_DEBUG, "Enc: %u - Going forward", encoderPos);
        		} else if (Ydir == -1 && targetReached) { //going forward and target reached
        			Ydir = 0;
        			doingYturn = false;
        			tel_Log(TEL_LOG_DEBUG, "Enc: %u - Going back to line", encoderPos);
        		}
        		servo_SetAngle(SERVO_FRONT, Ydir);
        		drv_SetSpeed(-Ydir*slowSpeed);


        	} else if(MAGIC_ENABLED && !lineResult.allBlack) { //motor enabled and on line - follow
                float control_signal = lc_Compute(-lineResult.detectedLinePos,
                                                    P_GAIN, //P 
                                                    I_GAIN, //I
                                                    D_GAIN, //D
                                                    0.1f);
                servo_SetAngle(SERVO_FRONT, control_signal);
                
                drv_SetSpeed(slowSpeed);
                drv_Enable(true);
            } else if  (MAGIC_ENABLED && lineResult.allBlack){ //finished sequence
            	drv_Enable(false); //stop
				pathIndex++;
				lineSplitIndex = 0;

            	encoderStart = encoderPos;
            	encoderTarget = encoderPos-encoderDiff;

            	doingYturn = true;
            	Ydir=1;
    			tel_Log(TEL_LOG_DEBUG, "Enc: %u - Going backward", encoderPos);
    			drv_Enable(true);

            } else {
            	// drv_Enable(false);
                drv_SetSpeed(0.0f);
                servo_SetAngle(SERVO_FRONT, 0.0f);
                lc_Init();
                lineSplitIndex = 0;
            }
        //}
    }
}

