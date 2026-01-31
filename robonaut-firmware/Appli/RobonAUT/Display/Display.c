/*
 * Display.cpp
 *
 *  Created on: Oct 9, 2025
 *      Author: Nagy Ákos
 */

#include "Display.h"
#include <assert.h>
#include "Camera/Camera.h"
#include "scrl.h"
#include "stm32_lcd.h"
#include "stm32_lcd_ex.h"
#include "stm32n6xx_nucleo.h"

#define SCREEN_HEIGHT (256)
#define SCREEN_WIDTH (256)
#define UTIL_LCD_COLOR_TRANSPARENT (0)

#define NUMBER_COLORS 10
const uint32_t colors[NUMBER_COLORS] = { UTIL_LCD_COLOR_GREEN,   UTIL_LCD_COLOR_RED,    UTIL_LCD_COLOR_CYAN,
                                         UTIL_LCD_COLOR_MAGENTA, UTIL_LCD_COLOR_YELLOW, UTIL_LCD_COLOR_GRAY,
                                         UTIL_LCD_COLOR_BLACK,   UTIL_LCD_COLOR_BROWN,  UTIL_LCD_COLOR_BLUE,
                                         UTIL_LCD_COLOR_ORANGE };

// foreground buffer
__attribute__((aligned(32))) uint8_t lcd_fg_buffer[SCREEN_WIDTH * SCREEN_HEIGHT * 2];

// screen buffer
__attribute__((aligned(32))) static uint8_t screen_buffer[SCREEN_WIDTH * SCREEN_HEIGHT * 2];

// camera buffer
extern uint8_t cam_full_frame_buffer[SCREEN_WIDTH * SCREEN_HEIGHT * 3];

bool DS_Init(void)
{
    SCRL_LayerConfig layers_config[2] = {
        {
            .origin = { 0, 0 },
            .size = { SCREEN_WIDTH, SCREEN_HEIGHT },
            .format = SCRL_RGB888,
            .address = cam_full_frame_buffer,
        },
        {
            .origin = { 0, 0 },
            .size = { SCREEN_WIDTH, SCREEN_HEIGHT },
            .format = SCRL_ARGB4444,
            .address = lcd_fg_buffer,
        },
    };

    SCRL_ScreenConfig screen_config = {
        .size = { SCREEN_WIDTH, SCREEN_HEIGHT },
        .format = SCRL_RGB565, // SCRL_RGB565 -> High FPS, less compatible (gstreamer) UVC | SCRL_YUV422 -> Low FPS,
                               // standard UVC camera
        .address = screen_buffer,
        .fps = 15,
    };

    /* Initialize the LCD to black */
    uint32_t* p_screen_buffer = (uint32_t*) screen_buffer;
    for (size_t i = 0; i < sizeof(screen_buffer) / 4; i++)
    {
        p_screen_buffer[i] = 0x80108010;
    }
    SCB_CleanDCache_by_Addr(screen_buffer, sizeof(screen_buffer));

    int ret = SCRL_Init((SCRL_LayerConfig* [2]) { &layers_config[0], &layers_config[1] }, &screen_config);
    assert(ret == 0);

    UTIL_LCD_SetLayer(SCRL_LAYER_1);
    UTIL_LCD_Clear(UTIL_LCD_COLOR_TRANSPARENT);
    UTIL_LCD_SetFont(&Font12);
    UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
    return true;
}

void DS_NetworkOutput(od_pp_out_t* pp_out)
{
    assert(pp_out);

    od_pp_outBuffer_t* rois = pp_out->pOutBuff;
    uint32_t nb_rois = pp_out->nb_detect;

    /* Draw bounding boxes */
    UTIL_LCD_FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, UTIL_LCD_COLOR_TRANSPARENT); /* Clear previous boxes */
    for (uint32_t i = 0; i < nb_rois; i++)
    {
        uint32_t x0 = (uint32_t) ((rois[i].x_center - rois[i].width / 2) * ((float32_t) SCREEN_WIDTH));
        uint32_t y0 = (uint32_t) ((rois[i].y_center - rois[i].height / 2) * ((float32_t) SCREEN_HEIGHT));
        uint32_t width = (uint32_t) (rois[i].width * ((float32_t) SCREEN_WIDTH));
        uint32_t height = (uint32_t) (rois[i].height * ((float32_t) SCREEN_HEIGHT));

        /* Draw boxes without going outside of the image */
        x0 = x0 < SCREEN_WIDTH ? x0 : SCREEN_WIDTH - 1;
        y0 = y0 < SCREEN_HEIGHT ? y0 : SCREEN_HEIGHT - 1;
        width = ((x0 + width) < SCREEN_WIDTH) ? width : (SCREEN_WIDTH - x0 - 1);
        height = ((y0 + height) < SCREEN_HEIGHT) ? height : (SCREEN_HEIGHT - y0 - 1);
        UTIL_LCD_DrawRect(x0, y0, width, height, colors[rois[i].class_index % NUMBER_COLORS]);
        UTIL_LCDEx_PrintfAt(-x0 - width, y0, RIGHT_MODE, "%d%%", (uint32_t) (rois[i].conf * 100.0f));
    }

    SRCL_Update();
}

uint8_t DS_IsReadyToUpdate(void)
{
    return SCRL_IsReadyToUpdate();
}

void DS_ResetState(void)
{
    SCRL_ResetState();
}
