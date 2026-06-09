#ifndef SSD1315_H
#define SSD1315_H

#include <stdint.h>
#include "asciiLib.h"
#include "hardware/i2c.h"

// The RP2040 uses 7-bit addresses natively. (0x78 >> 1 = 0x3C)
#define SSD1315_I2C_ADDR    0x3C

// Define the I2C port you are using on the Pico (i2c0 or i2c1)
#define OLED_I2C_PORT       i2c0



void SSD1315_Init(void);
void SSD1315_Clear(void);
void SSD1315_Update(void);
void SSD1315_DrawPixel(int x, int y, uint8_t color);
void SSD1315_DrawRect(int x, int y, int width, int height, uint8_t color);
void SSD1315_FillRect(int x, int y, int width, int height, uint8_t color);
void SSD1315_DrawChar8x16(int x, int y, char c, int font, uint8_t color);
void SSD1315_DrawString8x16(int x, int y, const char* str, int font, uint8_t color);
void SSD1315_DrawChar16x32(int x, int y, char c);
void SSD1315_DrawString16x32(int x, int y, const char* str);
void SSD1315_DrawLock(uint8_t x, uint8_t y, uint8_t state);

#endif