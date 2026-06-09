#pragma once
#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdint.h>

// B od strony koniczynki, A pod B
// B kolumny, A wiersze
#define COL_PINS_MASK ((1U << 6) | (1U << 3) | (1U << 4) | (1U << 5))
#define ROW_PINS_MASK ((1U << 26) | (1U << 27) | (1U << 28) | (1U << 29))
#define COL_PINS {6, 5, 4, 3}
#define ROW_PINS {26, 27, 28, 29}

#define IRQ_PIN 2

#define KEYPAD_LAYOUT { \
    {0x1, 0x2, 0x3, 0xA}, \
    {0x4, 0x5, 0x6, 0xB}, \
    {0x7, 0x8, 0x9, 0xC}, \
    {0x0, 0xF, 0xE, 0xD} \
}

#define KEY_ACCEPT 0xA
#define KEY_CANCEL 0xC

#define QUEUE_SIZE 8

static void keyEnqueue(uint8_t key);
uint8_t keyDequeue(uint8_t *key);
uint8_t keyPending();
static bool readColumn(repeating_timer_t *rt);
static void gpio_callback(uint gpio, uint32_t events);
void initKeypad();
