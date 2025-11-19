#include "../sys_interface.h"
#include "sys_main.h"

#include "npu_cache.h"
#include "NeuralNetwork/NeuralNetwork.h"
#include "LineSensor/LineSensor.h"
#include "MicroTimer/MicroTimer.h"
#include "SimpleLogger/SimpleLogger.h"

#include "SSD1306/ssd1306_interface.h"
#include "SSD1306/ssd1306_fonts.h"
#include "UserInput/ui_interface.h"
#include "Servo/servo_interface.h"
#include "MotorControl/mot_interface.h"
#include "HwTest/test_interface.h"
#include "Control/Control.h"
#include "LineProcessor/line_interface.h"
#include "ControllerTuning/tuning_interface.h"

#include <stdio.h>
#include <stdarg.h>


static tuning_ParametersType tuning_params;
static line_ParamSettingsType line_params;
line_DetectionResultType line_detection_result;

static void NPUCache_config(void);

void sys_Init(void)
{
    NPUCache_config();
    NN_Init();

    BspCOMInit.BaudRate = 921600;
    if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
        Error_Handler();
    if (!MT_Init(&htim6))
        Error_Handler();
    if (!LS_Init(&hspi4, NULL))
        Error_Handler();

    ssd1306_Init();
    ui_Init();
    servo_Init();
    mot_Init();
    CTRL_InitLoop();
    line_Init();
    tuning_Init(&tuning_params);

    test_Init();

    ssd1306_Fill(0);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("System Init Success", Font_6x8, 0);
    ssd1306_UpdateScreen();

    // Wait for user button press
    while (BSP_PB_GetState(BUTTON_USER) != GPIO_PIN_SET) { }
}


void sys_Run(void)
{
    // while(1)
    // {
    //     static uint32_t lastTestProcessTime = 0;
    //     if (HAL_GetTick() - lastTestProcessTime >= 100)
    //     {
    //         lastTestProcessTime = HAL_GetTick();
    //         test_ProcessAll();
    //     }
    // }

    float control_signal, speed;
    uint32_t lastProcessTime = 0;
    uint32_t lastLogTime = 0;
    uint32_t lastFastTime = 0, lastSlowTime = 0;
    uint8_t speed_mode = 0; // 0 -slow, 1 - fast
    while (1)
    {
        uint32_t currentTime = HAL_GetTick();
        if (currentTime - lastProcessTime < 10)
            continue;
        ui_Process();
        lastProcessTime = currentTime;
        line_Process();
        line_GetDetectionResult(&line_detection_result);
        printf("%f\n", line_detection_result.position);

        tuning_Process();

        line_params.adcThreshold = tuning_params.threshold;
		line_params.useSingleLineDetection = tuning_params.mode;
		line_SetParams(&line_params);
        if(line_detection_result.lineType == LINE_NO_LINE) {
        	mot_Enable(false);
            mot_SetSpeed(-tuning_params.speed);
            servo_SetAngle(SERVO_FRONT, 0.0f);
            tuning_Stop();
        } else {
            if(tuning_params.motor_enabled) {
                control_signal = CTRL_RunLoop_TUNING(-line_detection_result.position,
                                                    tuning_params.p_coeff, 
                                                    tuning_params.i_coeff, 
                                                    tuning_params.d_coeff, 
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
                        speed = -(tuning_params.speed*(1 - fmin(HAL_GetTick()-lastFastTime,200)/200.0f ));
                    else if (HAL_GetTick()-lastFastTime < 300)
                        speed = -(tuning_params.speed*(fmin(HAL_GetTick()-lastFastTime-200,100)/100.0f));
                    else 
                        speed = -(tuning_params.speed);

                    mot_SetSpeed(speed);
                } else if(speed_mode == 1){
                    lastFastTime = HAL_GetTick();
                    if(HAL_GetTick()-lastSlowTime < 400)
                        speed = -(tuning_params.speed);
                    else    
                        speed = -0.48f;
                    mot_SetSpeed(speed);
                }
                mot_Enable(true);
            } else {
            	mot_Enable(false);
                mot_SetSpeed(-tuning_params.speed);
                servo_SetAngle(SERVO_FRONT, 0.0f);
                CTRL_InitLoop();
            }
        }
    }
}


static void NPUCache_config(void)
{
    npu_cache_enable();
}

