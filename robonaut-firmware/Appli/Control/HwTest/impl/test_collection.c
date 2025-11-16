#include "../test_interface.h"

#include "LineSensor/LineSensor.h"
#include "MicroTimer/MicroTimer.h"
#include "SimpleLogger/SimpleLogger.h"

#include "SSD1306/ssd1306_interface.h"
#include "SSD1306/ssd1306_fonts.h"
#include "UserInput/ui_Interface.h"


void test_Init(void)
{
    // Initialize hardware tests here
}


void test_ProcessLineSensors(void)
{
    const uint16_t led_adc_threshold = 400;
    LS_ADC_Values_Type adc_values;
    LS_LED_Values_Type led_values;

    LS_Process();
    LS_GetADCValues(&adc_values);
    for (int i = 0; i < 32; i++)
    {
        led_values.front_led[i] = !(adc_values.front_adc[i] < led_adc_threshold);
        led_values.rear_led[i]  = !(adc_values.rear_adc[i]  < led_adc_threshold);
    }
    LS_SetFbLEDs(&led_values);
}


void test_ShowUiAndOLEDDemo(void)
{
    static int enterPressedCount = 0;
    static int backPressedCount = 0;
    static int knobPressedCount = 0;
    static uint32_t lastUiUpdateTime = 0;

    ui_Process();
    ui_StateType ui_state;
    ui_GetState(&ui_state);

    ssd1306_Fill(0);
    ssd1306_SetCursor(0, 0);

    enterPressedCount += ui_state.enterButtonWasPressed ? 1 : 0;
    backPressedCount  += ui_state.backButtonWasPressed  ? 1 : 0;
    knobPressedCount  += ui_state.knobButtonWasPressed  ? 1 : 0;

    uint32_t currentTime = HAL_GetTick();
    if (currentTime - lastUiUpdateTime < 100)
        return;
    lastUiUpdateTime = currentTime;

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Enc: %d", ui_state.encoderPos);
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_SetCursor(0, 10);
    snprintf(buffer, sizeof(buffer), "Enter: %d", enterPressedCount);
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_SetCursor(0, 20);
    snprintf(buffer, sizeof(buffer), "Back: %d", backPressedCount);
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_SetCursor(0, 30);
    snprintf(buffer, sizeof(buffer), "Knob: %d", knobPressedCount);
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_UpdateScreen();
}


void test_ProcessAll(void)
{
    test_ProcessLineSensors();
    test_ShowUiAndOLEDDemo();
}

