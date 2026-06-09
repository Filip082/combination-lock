#include "display.h"
#include <cstdio>

void init_display() {
    SSD1315_Init();
    SSD1315_Clear();
    SSD1315_DrawRect(0,0,127,63,1);
    SSD1315_DrawRect(2,2,125,61,1);
    SSD1315_DrawRect(2,21,125,23,1);

    //SSD1315_DrawString16x32(60, 27, "1347");
    SSD1315_DrawLock(18, 32, 0);

    SSD1315_Update();
}

void welcome_screen() {
    SSD1315_FillRect(4,25,120,35,0);
    const char *txt = "WELCOME";
    SSD1315_DrawString16x32(10, 27, txt);
    SSD1315_Update();
}

void print_address(uint16_t address) {
    SSD1315_FillRect(4,25,120,35,0);

    SSD1315_DrawLock(18, 32, 0);
    char txt[4];
    sprintf(txt, "%4d", address);
    SSD1315_DrawString16x32(60, 27, txt);
    SSD1315_Update();
}

void print_code(uint8_t digits[4], uint8_t num_digits){
    SSD1315_FillRect(4,25,120,35,0);
    SSD1315_DrawLock(18, 32, 0);
    char txt[5];
    for (int i = 0; i < num_digits; i++) {
        txt[i] = '0' + digits[i];
    }
    for (int i = num_digits; i < 4; i++) {
        txt[i] = '-';
    }
    txt[4] = '\0';
    SSD1315_DrawString16x32(60, 27, txt);
    SSD1315_Update();
}

void print_bad_code() {
    SSD1315_FillRect(4,25,120,35,0);
    const char *txt = "ZLY KOD";
    SSD1315_DrawString16x32(10, 27, txt);
    SSD1315_Update();
}

void print_open() {
    SSD1315_FillRect(4,25,120,35,0);
    const char *txt = "Otwarte";
    SSD1315_DrawString16x32(10, 27, txt);
    SSD1315_Update();
}

void configuration_error() {
    SSD1315_FillRect(4,25,120,35,0);
    const char *txt = "Bledna";
    const char *txt2 = "konfiguracja";
    SSD1315_DrawString8x16(10, 25, txt, 0, 1);
    SSD1315_DrawString8x16(10, 35, txt2, 0, 1);
    SSD1315_Update();
}
