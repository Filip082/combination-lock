#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
// #include <pico/i2c_slave.h>
#include <pico/time.h>
#include "tusb.h"

#include "leds.h"
#include "fram.h"
#include "keypad.h"
#include "common.h"
#include "display.h"
// extern "C" {
#include "rtc_functions.h"
// }

// I2C defines
// This example will use I2C0 on GPIO0 (SDA) and GPIO1 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1
// #define I2C_SLAVE_ADDRESS 0x67
#define I2C_BAUDRATE 400 * 1000

PIO pio;
uint sm;
uint offset;

struct config_block cfg;
enum state state_e = IDLE;
struct
{
    uint16_t address;
    uint8_t pin[4];
    uint8_t digitIndex;
} input;
flat user;

volatile bool usb_connected = false;

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void)itf;
    (void)rts;
    usb_connected = dtr;
}

bool receive_config(uint8_t *buf, size_t buf_size, size_t *received_len)
{
    while (true)
    {
        if (getchar_timeout_us(2000000) != 0xAA)
            continue;
        if (getchar_timeout_us(500000) != 0x55)
            continue;
        break;
    }

    uint8_t len_bytes[2];
    len_bytes[0] = getchar_timeout_us(500000);
    len_bytes[1] = getchar_timeout_us(500000);
    uint16_t len = len_bytes[0] | (len_bytes[1] << 8);

    if (len > buf_size)
        return false;

    for (uint16_t i = 0; i < len; i++)
        buf[i] = getchar_timeout_us(500000);

    putchar(0x06); // ACK
    *received_len = len;
    return true;
}

void printDate(void)
{
    datetime_t t;
    rtc_get_datetime(&t);
    char txt[20];
    sprintf(txt, "%02d-%02d-%02d %02d:%02d", t.day, t.month, t.year % 100, t.hour, t.min);

    SSD1315_DrawString8x16(8, 5, txt, 0, 1);
    SSD1315_Update();
}

uint8_t validateAddress()
{
    return input.address >= cfg.min_address && input.address <= cfg.max_address;
}

uint8_t validatePin()
{
    uint16_t pin = 0;
    for (int i = 0; i < 4; i++)
    {
        pin = pin << 4 | input.pin[i];
    }
    printf("Entered PIN: %04X, Expected: %04X\n", pin, user.pin);
    return pin == user.pin;
}

uint8_t getKey()
{
    uint8_t key;
    keyDequeue(&key);
    return key;
}

void setState(enum state newState)
{
    state_e = newState;

    uint8_t color = newState;
    put_pixel(pio, sm, colors[newState]);

    switch (newState)
    {
    case NO:
        print_bad_code();
        break;
    case IDLE:
        welcome_screen();
        break;
    case ADDRESS:
        print_address(input.address);
        break;
    case KEY:
        print_code(input.pin, 0);
        break;
    case CODE:
        print_code(input.pin, input.digitIndex);
        break;
    case YES:
        print_open();
        break;
    }
}

void keyPendingHandler()
{
    uint8_t key;
    key = getKey();

    if (key > 9)
    {
        if (key == KEY_CANCEL)
            setState(IDLE);
        if (key == KEY_ACCEPT && state_e == ADDRESS)
            setState(validateAddress() ? KEY : NO);
        return;
    }

    switch (state_e)
    {
    case NO:
    case IDLE:
        input.address = key;
        input.digitIndex = 0;
        setState(ADDRESS);
        break;

    case ADDRESS:
        input.address = input.address * 10 + key;
        setState(ADDRESS);
        break;

    case KEY:
        retrieve_user(&user, &cfg, input.address);
        setState(CODE);
        input.digitIndex = 0;
    case CODE:
        input.pin[input.digitIndex++] = key;
        print_code(input.pin, input.digitIndex);
        break;

    case YES:
    default:
        break;
    }

    if (input.digitIndex == 4)
    {
        setState(validatePin() ? YES : NO);
    }
}

int main()
{
    stdio_init_all();

    // This will find a free pio and state machine for our program and load it for us
    // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    put_pixel(pio, sm, 0);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, I2C_BAUDRATE);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    initKeypad();

    init_display();

    RTC_Init_dateTime();
    RTC_Init_Interrupt(printDate);
    printDate();

    if (read_config_from_fram(&cfg) == sizeof(config_block) && validate_config((const uint8_t *)&cfg, sizeof(config_block), true))
    {
        setState(IDLE);
    }
    else
    {
        setState(NO);
        configuration_error();
        while (!usb_connected)
        {
            tight_loop_contents();
        }
    }

    alarm_id_t alarm;

    while (true)
    {
        if (usb_connected)
        {
            uint8_t config_buffer[256];
            size_t config_len;
            if (receive_config(config_buffer, sizeof(config_buffer), &config_len) && validate_config(config_buffer, config_len))
            {
                put_pixel(pio, sm, save_config_to_fram(config_buffer, config_len) ? colors[YES] : colors[NO]);
                flat *flats = (flat *)(config_buffer + sizeof(config_block));
                for (size_t i = 0; i < ((config_len - sizeof(config_block)) / sizeof(flat)); i++)
                {
                    printf("Flat %u: address=%u, pin=%x\n", i, flats[i].address, flats[i].pin);
                }
                cfg = *(config_block *)config_buffer;
                setState(IDLE);
            }
            usb_connected = false;
        }
        if (keyPending())
        {
            cancel_alarm(alarm);
            keyPendingHandler();
            alarm = add_alarm_in_ms(5000, [](alarm_id_t id, void *user_data) -> int64_t
                                    {
                setState(IDLE);
                return 0; }, NULL, false);
        }
    }

    pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
}
