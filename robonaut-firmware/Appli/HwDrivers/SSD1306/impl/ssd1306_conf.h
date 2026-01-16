/**
 * Private configuration file for the SSD1306 library.
 * This example is configured for STM32F0, I2C and including all fonts.
 */

#ifndef __SSD1306_CONF_H__
#define __SSD1306_CONF_H__

// I2C Configuration
#define SSD1306_I2C_PORT hi2c1
#define SSD1306_I2C_ADDR (0x3C << 1)

#define SSD1306_HEIGHT 64
#define SSD1306_WIDTH 128

#define SSD1306_X_OFFSET_LOWER 2
#define SSD1306_X_OFFSET_UPPER 0

// Mirror the screen if needed
// #define SSD1306_MIRROR_VERT
// #define SSD1306_MIRROR_HORIZ

// Set inverse color if needed
// # define SSD1306_INVERSE_COLOR

// Set if DMA or IT I2C transfer should be used
#define SSD1306_I2C_IT

#define SSD1306_BUFFER_SIZE SSD1306_WIDTH* SSD1306_HEIGHT / 8
extern I2C_HandleTypeDef SSD1306_I2C_PORT;

#endif /* __SSD1306_CONF_H__ */
