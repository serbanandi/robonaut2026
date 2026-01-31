#include "sys_init.h"
#include "../sys_interface.h"
#include "main.h"
#include "spi.h"
#include "tim.h"

// #include "npu_cache.h"
// #include "NeuralNetwork/NeuralNetwork.h"

#include "AnalogInputs/anlg_interface.h"
#include "ControllerTuning/tuning_interface.h"
#include "Drive/drv_interface.h"
#include "HwTest/test_interface.h"
#include "IMU/imu_interface.h"
#include "LineController/lc_interface.h"
#include "LineProcessor/line_interface.h"
#include "LineSensor/ls_interface.h"
#include "MicroTimer/mt_interface.h"
#include "RadioControl/rc_interface.h"
#include "SSD1306/ssd1306_fonts.h"
#include "SSD1306/ssd1306_interface.h"
#include "Servo/servo_interface.h"
#include "Telemetry/tel_interface.h"
#include "UserInput/ui_interface.h"

static drv_ControlParamsType telVar_currentDrvParams = {
    .P = 1.6f,
    .I = 3.00f,
    .D = 0.00f,
    .integralLimit = 0.5f,
    .periodUs = 1000 // 1 ms
};
static float telVar_currentMaxPower = 0.4f;
static uint32_t telVar_maxEncoderCps = 300000; // 300k counts per second for now, TODO: tune properly

static uint32_t telVar_encoderPos = 0;
static float telVar_encoderSpeed = 0.0f;

void sys_Init(void)
{
    // npu_cache_enable();
    // NN_Init();

    mt_Init();
    tel_Init();
    ls_Init();
    ssd1306_Init();
    ui_Init();
    servo_Init();
    drv_Init(&telVar_currentDrvParams, telVar_currentMaxPower, telVar_maxEncoderCps);
    line_Init();
    anlg_Init();
    rc_Init();
    // imu_init(&_sys_imuInstance, &hspi2, IMU_CS_GPIO_Port, IMU_CS_Pin,
    // SPI2_IRQn, &htim18); imu_setDefaultSettings(&_sys_imuInstance);

    // test_Init();

    _sys_RegisterTelemetryVariables();

    HAL_Delay(500); // Wait for everything to stabilize
    ssd1306_Fill(0);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("System Init Success", Font_6x8, 0);
    ssd1306_UpdateScreen();
    tel_Log(TEL_LOG_INFO, "System initialized successfully.");
}

void _sys_RegisterTelemetryVariables(void)
{
    tel_RegisterRW(&telVar_currentDrvParams.P, TEL_FLOAT, "drv_P", 1000);
    tel_RegisterRW(&telVar_currentDrvParams.I, TEL_FLOAT, "drv_I", 1000);
    tel_RegisterRW(&telVar_currentDrvParams.D, TEL_FLOAT, "drv_D", 1000);
    tel_RegisterRW(&telVar_currentDrvParams.integralLimit, TEL_FLOAT, "drv_integralLimit", 1000);
    tel_RegisterRW(&telVar_currentDrvParams.periodUs, TEL_UINT32, "drv_periodUs", 1000);
    tel_RegisterRW(&telVar_currentMaxPower, TEL_FLOAT, "drv_maxPower", 1000);
    tel_RegisterRW(&telVar_maxEncoderCps, TEL_UINT32, "drv_maxEncoderCps", 1000);
    tel_RegisterR(&telVar_encoderPos, TEL_UINT32, "drv_encoderPos", 200);
    tel_RegisterR(&telVar_encoderSpeed, TEL_FLOAT, "drv_encoderSpeed", 200);
}

void _sys_HandleParamTuning(void)
{
    drv_SetControlParams(&telVar_currentDrvParams);
    drv_SetMaxPower(telVar_currentMaxPower);
    drv_SetMaxEncoderCps(telVar_maxEncoderCps);
    telVar_encoderPos = drv_GetEncoderCount();
    telVar_encoderSpeed = drv_GetEncoderSpeed();
}
