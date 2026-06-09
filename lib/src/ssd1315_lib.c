#include "ssd1315_lib.h"
#include "asciiLib.h"
#include <string.h>
#include "pico/stdlib.h"

static uint8_t RAM_Buffer[1024];

//16x10
const uint16_t lock_handle[10] = {
    0x03C0, 
    0x0FF0, 
    0x3C3C, 
    0x300C,
    0x6006, 
    0x6006, 
    0xC003, 
    0xC003, 
    0xC003, 
    0xC003
};

static void write_cmd(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    // i2c_write_blocking handles the wait automatically on the RP2040
    i2c_write_blocking(OLED_I2C_PORT, SSD1315_I2C_ADDR, buf, 2, false);
}

void SSD1315_Init(void) {
    write_cmd(0xAE); // Display Off
    write_cmd(0xD5); write_cmd(0x80); // Set Clock Divide Ratio
    write_cmd(0xA8); write_cmd(0x3F); // Set Multiplex Ratio (1/64)
    write_cmd(0xD3); write_cmd(0x00); // Display Offset
    write_cmd(0x40); // Start Line 0
    write_cmd(0x8D); write_cmd(0x14); // Enable Charge Pump (CRUCIAL)
    write_cmd(0x20); write_cmd(0x00); // Horizontal Addressing Mode
    write_cmd(0xA1); // Segment Remap
    write_cmd(0xC8); // COM Output Scan Direction
    write_cmd(0xDA); write_cmd(0x12); // COM Pins Hardware Config
    write_cmd(0x81); write_cmd(0xCF); // Contrast Control
    write_cmd(0xD9); write_cmd(0xF1); // Pre-charge Period
    write_cmd(0xDB); write_cmd(0x40); // VCOMH Deselect Level
    write_cmd(0xA4); // Entire Display ON (Resume)
    write_cmd(0xA6); // Normal Display
    write_cmd(0xAF); // Display ON
    
    SSD1315_Clear();
    SSD1315_Update();
}

void SSD1315_Clear(void) {
    memset(RAM_Buffer, 0, sizeof(RAM_Buffer));
}

void SSD1315_DrawPixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;
    
    if (color) {
        RAM_Buffer[x + (y / 8) * 128] |= (1 << (y % 8));
    } else {
        RAM_Buffer[x + (y / 8) * 128] &= ~(1 << (y % 8));
    }
}

void SSD1315_Update(void) {
    // 1. Tell the OLED we are updating the whole screen
    write_cmd(0x21); write_cmd(0); write_cmd(127); // Column Address bounds
    write_cmd(0x22); write_cmd(0); write_cmd(7);   // Page Address bounds
    
    // 2. Send the buffer in 8 smaller chunks (pages)
    for(int i = 0; i < 8; i++) {
        uint8_t chunk_buf[129];
        
        // Control byte: 0x40 means "Incoming Data"
        chunk_buf[0] = 0x40; 
        
        // Copy 128 bytes from our RAM buffer into this chunk
        memcpy(&chunk_buf[1], &RAM_Buffer[i * 128], 128);
        
        // Transmit the 129-byte chunk directly
        i2c_write_blocking(OLED_I2C_PORT, SSD1315_I2C_ADDR, chunk_buf, 129, false);
    }
}

void SSD1315_DrawRect(int x, int y, int x2, int y2, uint8_t color) {
    for (int i = x; i <= x2; i++) {
        SSD1315_DrawPixel(i, y, color);
        SSD1315_DrawPixel(i, y2, color);
    }
    for (int i = y; i <= y2; i++) {
        SSD1315_DrawPixel(x, i, color);
        SSD1315_DrawPixel(x2, i, color);
    }
}

void SSD1315_FillRect(int x, int y, int width, int height, uint8_t color) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            SSD1315_DrawPixel(x + i, y + j, color);
        }
    }
}

void SSD1315_DrawChar8x16(int x, int y, char c, int font, uint8_t color) {
    if (c < ' ' || c > '~') return; 
    unsigned char buffer[16];
    
    GetASCIICode(font, buffer, c);

    for (int row = 0; row < 16; row++) {
        uint8_t row_data = buffer[row];
        
        for (int col = 0; col < 8; col++) {
            if (row_data & (0x80 >> col)) {
                SSD1315_DrawPixel(x + col, y + row, color);
            } else {
                SSD1315_DrawPixel(x + col, y + row, 0);
            }
        }
    }
}

void SSD1315_DrawString8x16(int x, int y, const char* str, int font, uint8_t color) {
    int current_x = x;
    int current_y = y;
    while (*str) {
        SSD1315_DrawChar8x16(current_x, current_y, *str, font, color);
        current_x += 8;
        str++;
    }
}

void SSD1315_DrawChar16x32(int x, int y, char c) {
    int font = 0;
    uint8_t color = 1;
    if (c < ' ' || c > '~') return; 
    unsigned char buffer[16];
    
    GetASCIICode(font, buffer, c);

    for (int row = 0; row < 16; row++) {
        uint8_t row_data = buffer[row];
        
        for (int col = 0; col < 8; col++) {
            if (row_data & (0x80 >> col)) color = 1;
            else color = 0; 
            
            SSD1315_DrawPixel(x + col*2, y + row*2, color);
            SSD1315_DrawPixel(x + col*2+1, y + row*2, color);
            SSD1315_DrawPixel(x + col*2, y + row*2+1, color);
            SSD1315_DrawPixel(x + col*2+1, y + row*2+1, color);
        }
    }
}

void SSD1315_DrawString16x32(int x, int y, const char* str) {
    int current_x = x;
    int current_y = y;
    while (*str) {
        SSD1315_DrawChar16x32(current_x, current_y, *str);
        current_x += 16;
        str++;
    }
}

void SSD1315_DrawLock(uint8_t x, uint8_t y, uint8_t state) {
    SSD1315_FillRect(x, y, 18, 10 , 0);
    SSD1315_FillRect(x, y+10, 18, 12 ,1);
    x += 1;
    if (state == 1) x += 14;
    for (uint8_t i = 0; i < 10; i++){
        uint16_t row = lock_handle[i];
        for (uint8_t j = 0; j < 16; j++){
            SSD1315_DrawPixel(x+j, y+i, (row & (1 << (15-j))) ? 1 : 0);
        }
    }
}