#include "../test_interface.h"

#include "LineSensor/LineSensor.h"
#include "MicroTimer/MicroTimer.h"
#include "SimpleLogger/SimpleLogger.h"

#include "SSD1306/ssd1306_interface.h"
#include "SSD1306/ssd1306_fonts.h"
#include "UserInput/ui_interface.h"
#include "Servo/servo_interface.h"
#include "MotorControl/mot_interface.h"


static bool motorEnabled = false;


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
    ui_GetButtonState(&ui_state);

    ssd1306_Fill(0);
    ssd1306_SetCursor(0, 0);

    enterPressedCount += ui_state.enterButtonWasPressed ? 1 : 0;
    backPressedCount  += ui_state.backButtonWasPressed  ? 1 : 0;
    knobPressedCount  += ui_state.knobButtonWasPressed  ? 1 : 0;

    if (ui_state.enterButtonWasPressed)
    {
        motorEnabled = !motorEnabled;
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
    snprintf(buffer, sizeof(buffer), "Enc: %d\n", ui_GetEncoderPosition());
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
    snprintf(buffer, sizeof(buffer), "Mot: %s\n", motorEnabled ? "On" : "Off");
    ssd1306_WriteString(buffer, Font_6x8, 0);
    ssd1306_UpdateScreen();
}


void test_ProcessServoTest(void)
{
    static servo_SelectType current_servo = SERVO_FRONT;
    
    //set the servo position using the encoder position
    int32_t encoder_pos = ui_GetEncoderPosition();
    float servo_pos = encoder_pos * 0.05f;

    if (!motorEnabled)
        servo_SetAngle(current_servo, servo_pos);

    // Switch between max and min positions every 5 seconds
    // static uint32_t last_switch_time = 0;
    // static bool position_max = false;
    // uint32_t current_time = HAL_GetTick();

    // if (current_time - last_switch_time >= 2000 && !motorEnabled)
    // {
    //     position_max = !position_max;
    //     float servo_pos = position_max ? 1.0f : -1.0f;
    //     servo_SetAngle(current_servo, servo_pos);

    //     last_switch_time = current_time;
    // }
}


void test_ProcessMotorTest(void)
{
    if (motorEnabled)
    {
        // Set motor speed based on encoder position
        int32_t encoder_pos = ui_GetEncoderPosition();
        float motor_speed = encoder_pos * 0.02f; // Scale factor for speed
        mot_Enable(true);
        mot_SetSpeed(motor_speed);
    }
    else
    {
        mot_Enable(false);
    }
}


void test_ProcessAll(void)
{
    test_ProcessLineSensors();
    test_ProcessServoTest();
    test_ShowUiAndOLEDDemo();
    test_ProcessMotorTest();
}

