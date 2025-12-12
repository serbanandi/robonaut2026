/**
 * This Library was originally written by Olivier Van den Eede (4ilo) in 2016.
 * Some refactoring was done and SPI support was added by Aleksander Alekseev (afiskon) in 2018.
 *
 * https://github.com/afiskon/stm32-ssd1306
 */

#ifndef __SSD1306_H__
#define __SSD1306_H__

#include <stddef.h>
#include <stdint.h>
#include <_ansi.h>
#include <stdbool.h>

_BEGIN_STD_C

#include "stm32n6xx_hal.h"

typedef struct {
    uint8_t x;
    uint8_t y;
} ssd1306_Vertex;

/** Font */
typedef struct {
	const uint8_t width;                /**< Font width in pixels */
	const uint8_t height;               /**< Font height in pixels */
	const uint16_t *const data;         /**< Pointer to font data array */
    const uint8_t *const char_width;    /**< Proportional character width in pixels (NULL for monospaced) */
} SSD1306_Font_t;

// Procedure definitions
void ssd1306_Init(void);
void ssd1306_Fill(bool color);
bool ssd1306_UpdateScreen(void);
void ssd1306_WaitForScreenUpdateCplt(void);
void ssd1306_DrawPixel(uint8_t x, uint8_t y, bool color);
char ssd1306_WriteChar(char ch, SSD1306_Font_t Font, bool color);
char ssd1306_WriteString(const char* str, SSD1306_Font_t Font, bool color);
void ssd1306_SetCursor(uint8_t x, uint8_t y);
void ssd1306_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool color);
void ssd1306_DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, bool color);
void ssd1306_DrawArcWithRadiusLine(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, bool color);
void ssd1306_DrawCircle(uint8_t par_x, uint8_t par_y, uint8_t par_r, bool color);
void ssd1306_FillCircle(uint8_t par_x,uint8_t par_y,uint8_t par_r,bool par_color);
void ssd1306_Polyline(const ssd1306_Vertex *par_vertex, uint16_t par_size, bool color);
void ssd1306_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool color);
void ssd1306_FillRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool color);

/**
 * @brief Invert color of pixels in rectangle (include border)
 * 
 * @param x1 X Coordinate of top left corner
 * @param y1 Y Coordinate of top left corner
 * @param x2 X Coordinate of bottom right corner
 * @param y2 Y Coordinate of bottom right corner
 * @return SSD1306_Error_t status
 */
uint8_t ssd1306_InvertRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

void ssd1306_DrawBitmap(uint8_t x, uint8_t y, const unsigned char* bitmap, uint8_t w, uint8_t h, bool color);

/**
 * @brief Sets the contrast of the display.
 * @param[in] value contrast to set.
 * @note Contrast increases as the value increases.
 * @note RESET = 7Fh.
 */
void ssd1306_SetContrast(const uint8_t value);

_END_STD_C

#endif // __SSD1306_H__
