#pragma once
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "config.h"

#define FRAM_ADDR 0x50
#define I2C_PORT i2c0

uint8_t save_config_to_fram(const uint8_t *buf, size_t len);
uint8_t read_config_from_fram(struct config_block *cfg);
uint8_t retrieve_user(struct flat *user, struct config_block *cfg, uint16_t address);
