#pragma once
extern "C" {
    #include "ssd1315_lib.h" 
}
#include "pico/stdlib.h"

void init_display();
void welcome_screen();
void print_address(uint16_t address);
void print_code(uint8_t digits[4], uint8_t num_digits);
void print_bad_code();
void print_open();
void configuration_error();
