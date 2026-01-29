#include "../test_interface.h"

#include <stdio.h>
#include "AnalogInputs/anlg_interface.h"
#include "Drive/drv_interface.h"
#include "LineSensor/ls_interface.h"
#include "MicroTimer/mt_interface.h"
#include "RadioControl/rc_interface.h"
#include "SSD1306/ssd1306_fonts.h"
#include "SSD1306/ssd1306_interface.h"
#include "Servo/servo_interface.h"
#include "Telemetry/tel_interface.h"
#include "UserInput/ui_interface.h"

static bool telVar_motorEnabled = false;
static float telVar_fontServoTestAngle = 0.0f;
static float telVar_rearServoTestAngle = 0.0f;
static float telVar_motorTestSpeed = 0.0f;
static uint16_t telVar_lineLedAdcThr = 700;

void test_Init(void)
{
    tel_RegisterRW(&telVar_motorEnabled, TEL_UINT8, "test_motorEnabled", 1000);
    tel_RegisterRW(&telVar_fontServoTestAngle, TEL_FLOAT, "test_fontServoTestAngle", 1000);
    tel_RegisterRW(&telVar_rearServoTestAngle, TEL_FLOAT, "test_rearServoTestAngle", 1000);
    tel_RegisterRW(&telVar_motorTestSpeed, TEL_FLOAT, "test_motorTestSpeed", 1000);
    tel_RegisterRW(&telVar_lineLedAdcThr, TEL_UINT16, "test_lineLedAdcThr", 1000);
    tel_Log(TEL_LOG_INFO, "HwTest module initialized.");
}

void test_ProcessLineSensors(void)
{
    ls_AdcValuesType adc_values[2];
    ls_LedValuesType led_values[2];

    bool newDataAvailable = ls_IsNewDataAvailable();
    if (!newDataAvailable)
        return;
    ls_ClearNewDataFlag();

    ls_GetADCValues(&adc_values[LS_SENSOR_FRONT], LS_SENSOR_FRONT);
    ls_GetADCValues(&adc_values[LS_SENSOR_REAR], LS_SENSOR_REAR);
    for (int i = 0; i < 32; i++)
    {
        led_values[LS_SENSOR_FRONT].v[i] = !(adc_values[LS_SENSOR_FRONT].v[i] < telVar_lineLedAdcThr);
        led_values[LS_SENSOR_REAR].v[i] = !(adc_values[LS_SENSOR_REAR].v[i] < telVar_lineLedAdcThr);
    }
    ls_SetFbLEDs(&led_values[LS_SENSOR_FRONT], LS_SENSOR_FRONT);
    ls_SetFbLEDs(&led_values[LS_SENSOR_REAR], LS_SENSOR_REAR);
}

void test_ShowUiAndOLEDDemo(void)
{
    static int enterPressedCount = 0;
    static int backPressedCount = 0;
    static int knobPressedCount = 0;
    static uint32_t lastUiUpdateTime = 0;

    ui_StateType ui_state;
    ui_GetButtonState(&ui_state);

    ssd1306_Fill(0);
    ssd1306_SetCursor(0, 0);

    enterPressedCount += ui_state.enterButtonWasPressed ? 1 : 0;
    backPressedCount += ui_state.backButtonWasPressed ? 1 : 0;
    knobPressedCount += ui_state.knobButtonWasPressed ? 1 : 0;

    if (ui_state.enterButtonWasPressed || ui_state.backButtonWasPressed || ui_state.knobButtonWasPressed)
    {
        telVar_motorEnabled = false;
    }

    uint32_t currentTime = HAL_GetTick();
    if (currentTime - lastUiUpdateTime < 100)
        return;
    lastUiUpdateTime = currentTime;

    char buffer[64];

    snprintf(buffer, sizeof(buffer), "Time: %lu\n", currentTime / 1000);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString(buffer, Font_7x10, 0);

    ssd1306_SetCursor(0, 20);
    snprintf(buffer, sizeof(buffer), "Enc: %ld\n", ui_GetEncoderPosition());
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_SetCursor(0, 30);
    snprintf(buffer, sizeof(buffer), "Enter: %d\n", enterPressedCount);
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_SetCursor(0, 40);
    snprintf(buffer, sizeof(buffer), "Back: %d\n", backPressedCount);
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_SetCursor(0, 50);
    snprintf(buffer, sizeof(buffer), "Knob: %d\n", knobPressedCount);
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_SetCursor(80, 50);
    snprintf(buffer, sizeof(buffer), "Mot: %s\n", telVar_motorEnabled ? "On" : "Off");
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_UpdateScreen();
}

void test_ShowAdcAndRcAndRadioDemo(void)
{
    static uint32_t lastUpdateTime = 0;
    uint32_t currentTime = HAL_GetTick();
    if (currentTime - lastUpdateTime < 100)
        return;
    lastUpdateTime = currentTime;

    anlg_BatteryStatusType batteryStatus;
    anlg_DistancesType distances;
    anlg_ReadBatteryStatus(&batteryStatus);
    anlg_ReadDistances(&distances);

    static rc_PositionType rcPosition = { .fromNode = 'X', .toNode = 'X', .nextNode = 'X', .positionPercent = 0 };
    rc_RxStateType rcState;
    rc_GetPosition(&rcPosition);
    rcState = rc_GetRxState();

    bool isTriggerPulled = ui_GetRCTriggerState();

    ssd1306_Fill(0);
    ssd1306_SetCursor(0, 0);
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Aux: %.1fV Mot: %.2fV\n", batteryStatus.auxBatteryVoltage,
             batteryStatus.motorBatteryVoltage);
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_SetCursor(0, 10);
    snprintf(buffer, sizeof(buffer), "D: %lu,%lu,%lu,%lu", distances.channel[0], distances.channel[1],
             distances.channel[2], distances.channel[3]);
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_SetCursor(0, 20);
    snprintf(buffer, sizeof(buffer), "RC State: %d", rcState);
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_SetCursor(0, 30);
    snprintf(buffer, sizeof(buffer), "From: %c To: %c", rcPosition.fromNode, rcPosition.toNode);
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_SetCursor(0, 40);
    snprintf(buffer, sizeof(buffer), "Next: %c Prog: %d%%", rcPosition.nextNode, rcPosition.positionPercent);
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_SetCursor(0, 50);
    snprintf(buffer, sizeof(buffer), "Trig: %s", isTriggerPulled ? "Pulled" : "Released");
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_UpdateScreen();

    // ssd1306_SetCursor(0, 0);
    // ssd1306_WriteString("RC State:", Font_6x8, 0);
    // char buf[20];
    // sprintf(buf, "%d", rc_currentRxState);
    // ssd1306_SetCursor(80, 0);
    // ssd1306_WriteString(buf, Font_6x8, 0);
    // ssd1306_SetCursor(0, 20);
    // ssd1306_WriteString("From:", Font_6x8, 0);
    // sprintf(buf, "%c", latestPosition.fromNode);
    // ssd1306_SetCursor(50, 20);
    // ssd1306_WriteString(buf, Font_6x8, 0);
    // ssd1306_SetCursor(0, 30);
    // ssd1306_WriteString("To:", Font_6x8, 0);
    // sprintf(buf, "%c", latestPosition.toNode);
    // ssd1306_SetCursor(50, 30);
    // ssd1306_WriteString(buf, Font_6x8, 0);
    // ssd1306_SetCursor(0, 40);
    // ssd1306_WriteString("Next:", Font_6x8, 0);
    // sprintf(buf, "%c", latestPosition.nextNode);
    // ssd1306_SetCursor(50, 40);
    // ssd1306_WriteString(buf, Font_6x8, 0);
    // ssd1306_SetCursor(0, 50);
    // ssd1306_WriteString("Progress:", Font_6x8, 0);
    // sprintf(buf, "%d", latestPosition.positionPercent);
    // ssd1306_SetCursor(60, 50);
    // ssd1306_WriteString(buf, Font_6x8, 0);
    // ssd1306_UpdateScreen();
}

void test_ProcessServoTest(void)
{
    servo_SetAngle(SERVO_FRONT, telVar_fontServoTestAngle);
    servo_SetAngle(SERVO_BACK, telVar_rearServoTestAngle);
}

void test_ProcessMotorTest(void)
{
    drv_Enable(telVar_motorEnabled);
    drv_SetSpeed(telVar_motorTestSpeed);
}

void test_ProcessAll(void)
{
    test_ProcessLineSensors();
    test_ProcessServoTest();
    // test_ShowUiAndOLEDDemo();
    test_ShowAdcAndRcAndRadioDemo();
    test_ProcessMotorTest();
}
