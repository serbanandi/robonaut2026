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


static tuning_ParametersType tuning_params;
static line_ParamSettingsType line_params;
static line_DetectionResultType line_detection_result;

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
    float control_signal;
    while (1)
    {
        line_Process();
        line_GetDetectionResult(&line_detection_result);
        tuning_Process();
        if (HAL_GetTick() % 10 == 0) {
            if(tuning_params.motor_enabled) {
                line_params.adcThreshold = tuning_params.threshold;
                line_params.useSingleLineDetection = tuning_params.mode;
                line_SetParams(&line_params);

                control_signal = CTRL_RunLoop_TUNING(line_GetLastDetectedLinePos(), 
                                                     tuning_params.p_coeff, 
                                                     tuning_params.i_coeff, 
                                                     tuning_params.d_coeff, 
                                                     0.01f);
                servo_SetAngle(SERVO_FRONT, control_signal);
                
                mot_SetSpeed(tuning_params.speed);
                mot_Enable(true);
            } else {
                mot_Enable(false);
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

