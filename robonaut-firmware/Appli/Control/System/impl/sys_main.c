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

line_DetectionResultType line_detection_result;
bool MAGIC_ENABLED = false;
float slowSpeed = 0.3f;
float fastSpeed = 0.7f;

void sys_Run(void)
{
    static uint32_t encoderPos;
    static float encoderSpeed;

    tel_RegisterR(&encoderPos, TEL_UINT32, "sys_encoderPos", 100);
    tel_RegisterR(&encoderSpeed, TEL_FLOAT, "sys_encoderSpeed", 100);
    tel_RegisterRW(&MAGIC_ENABLED, TEL_UINT8, "sys_ENABLE", 400);
    tel_RegisterRW(&slowSpeed, TEL_FLOAT, "sys_slowSpeed", 400);
    tel_RegisterRW(&fastSpeed, TEL_FLOAT, "sys_fastSpeed", 400);


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

    float control_signal, speed;
    uint32_t lastProcessTime = 0;
    uint32_t lastFastTime = 0, lastSlowTime = 0;
    uint8_t speed_mode = 0; // 0 -slow, 1 - fast
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

        uint32_t currentTime = HAL_GetTick();
        if (currentTime - lastProcessTime < 10)
            continue;
        lastProcessTime = currentTime;
        line_Process();
        line_GetDetectionResult(&line_detection_result);
        
        if(line_detection_result.lineType == LINE_NO_LINE) {
        	drv_Enable(false);
            drv_SetSpeed(slowSpeed);
            servo_SetAngle(SERVO_FRONT, 0.0f);
        } else {
            if(MAGIC_ENABLED) {
                control_signal = lc_Compute(-line_detection_result.position,
                                                    0.105f, //P 
                                                    0.005f, //I
                                                    0.32f,  //D
                                                    0.1f);
                servo_SetAngle(SERVO_FRONT, control_signal);
                
                if (line_detection_result.lineType == LINE_TRIPLE_LINE_DASHED) 
                    speed_mode = 1; //fast
                else if (line_detection_result.lineType == LINE_TRIPLE_LINE) 
                    speed_mode = 0; //slow
                
                //mot_SetSpeed(-tuning_params.speed);
                if (speed_mode == 0){
                    lastSlowTime = HAL_GetTick();
                    if (HAL_GetTick()-lastFastTime < 200)
                        speed = slowSpeed*(1 - fmin(HAL_GetTick()-lastFastTime,200)/200.0f );
                    else if (HAL_GetTick()-lastFastTime < 300)
                        speed = slowSpeed*(fmin(HAL_GetTick()-lastFastTime-200,100)/100.0f);
                    else 
                        speed = slowSpeed;

                    drv_SetSpeed(speed);
                } else if(speed_mode == 1){
                    lastFastTime = HAL_GetTick();
                    if(HAL_GetTick()-lastSlowTime < 400)
                        speed = slowSpeed;
                    else    
                        speed = fastSpeed;
                    drv_SetSpeed(speed);
                }
                drv_Enable(true);
            } else {
            	drv_Enable(false);
                drv_SetSpeed(slowSpeed);
                servo_SetAngle(SERVO_FRONT, 0.0f);
                lc_Init();
            }
        }
    }
}

