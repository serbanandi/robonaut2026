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
#include "HwTest/test_interface.h"


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
    CTRL_InitLoop();

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
    while (1)
    {
        test_ProcessAll();
    }
}


static void NPUCache_config(void)
{
    npu_cache_enable();
}

