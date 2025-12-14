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
#include "ControllerTuning/tuning_interface.h"
#include "Control/Control.h"

#include <stdio.h>
#include <stdarg.h>

static tuning_ParametersType tuning_params;
static line_ParamSettingsType line_params;
line_DetectionResultType line_detection_result;

void sys_Run(void)
{
    static uint32_t encoderPos;
    static float encoderSpeed;

    tel_RegisterR(&encoderPos, TEL_UINT32, "Encoder_Position", 100);
    tel_RegisterR(&encoderSpeed, TEL_FLOAT, "Encoder_Speed", 100);

    tel_Log(TEL_LOG_INFO, "Entering main loop...");

    while (1)
    {
        _sys_HandleParamTuning();
        encoderPos = drv_GetEncoderCount();
        encoderSpeed = drv_GetEncoderSpeed();
        
        test_ProcessAll();
        tel_Process();

        static uint32_t lastBlinkTime = 0;
        if (HAL_GetTick() - lastBlinkTime >= 500)
        {
            lastBlinkTime = HAL_GetTick();
            BSP_LED_Toggle(LED1);
        }
    }

    // Example loop that receives data over UART and echoes it back in the form: "Received: <data>\n"
//    uint8_t rxBuffer[128];
//    size_t receivedSize = 0;
//    size_t currentRecvSize = 0;
//    uint8_t txBuffer[256];
//    while (1)
//    {
//        uint8_t result = uart_Receive(&tel_uart, rxBuffer + receivedSize, sizeof(rxBuffer) - receivedSize, &currentRecvSize);
//        receivedSize += currentRecvSize;
//        if (result)
//        {
//            // Prepare the response
//            int n = snprintf((char*)txBuffer, sizeof(txBuffer), "Received: %.*s\n", (int)receivedSize, rxBuffer);
//            // Transmit the response
//            uart_Transmit(&tel_uart, txBuffer, n);
//            receivedSize = 0; // Reset for next reception
//        }
//        else if (receivedSize >= sizeof(rxBuffer))
//        {
//            // Buffer full without receiving termination character, reset buffer
//            receivedSize = 0;
//        }
//    }

    // float control_signal, speed;
    // uint32_t lastProcessTime = 0;
    // uint32_t lastLogTime = 0;
    // uint32_t lastFastTime = 0, lastSlowTime = 0;
    // uint8_t speed_mode = 0; // 0 -slow, 1 - fast
    // while (1)
    // {
    //     uint32_t currentTime = HAL_GetTick();
    //     if (currentTime - lastProcessTime < 10)
    //         continue;
    //     ui_Process();
    //     lastProcessTime = currentTime;
    //     line_Process();
    //     line_GetDetectionResult(&line_detection_result);
    //     printf("%f\n", line_detection_result.position);

    //     tuning_Process();

    //     line_params.adcThreshold = tuning_params.threshold;
	// 	line_params.useSingleLineDetection = tuning_params.mode;
	// 	line_SetParams(&line_params);
    //     if(line_detection_result.lineType == LINE_NO_LINE) {
    //     	mot_Enable(false);
    //         mot_SetSpeed(-tuning_params.speed);
    //         servo_SetAngle(SERVO_FRONT, 0.0f);
    //         tuning_Stop();
    //     } else {
    //         if(tuning_params.motor_enabled) {
    //             control_signal = CTRL_RunLoop_TUNING(-line_detection_result.position,
    //                                                 tuning_params.p_coeff, 
    //                                                 tuning_params.i_coeff, 
    //                                                 tuning_params.d_coeff, 
    //                                                 0.1f);
    //             servo_SetAngle(SERVO_FRONT, control_signal);
                
    //             if (line_detection_result.lineType == LINE_TRIPLE_LINE_DASHED) 
    //                 speed_mode = 1; //fast
    //             else if (line_detection_result.lineType == LINE_TRIPLE_LINE) 
    //                 speed_mode = 0; //slow
                
    //             //mot_SetSpeed(-tuning_params.speed);
    //             if (speed_mode == 0){
    //                 lastSlowTime = HAL_GetTick();
    //                 if (HAL_GetTick()-lastFastTime < 200)
    //                     speed = -(tuning_params.speed*(1 - fmin(HAL_GetTick()-lastFastTime,200)/200.0f ));
    //                 else if (HAL_GetTick()-lastFastTime < 300)
    //                     speed = -(tuning_params.speed*(fmin(HAL_GetTick()-lastFastTime-200,100)/100.0f));
    //                 else 
    //                     speed = -(tuning_params.speed);

    //                 mot_SetSpeed(speed);
    //             } else if(speed_mode == 1){
    //                 lastFastTime = HAL_GetTick();
    //                 if(HAL_GetTick()-lastSlowTime < 400)
    //                     speed = -(tuning_params.speed);
    //                 else    
    //                     speed = -0.48f;
    //                 mot_SetSpeed(speed);
    //             }
    //             mot_Enable(true);
    //         } else {
    //         	mot_Enable(false);
    //             mot_SetSpeed(-tuning_params.speed);
    //             servo_SetAngle(SERVO_FRONT, 0.0f);
    //             CTRL_InitLoop();
    //         }
    //     }
    // }
}

