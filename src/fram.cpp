#include "fram.h"

uint8_t save_config_to_fram(const uint8_t *buf, size_t len) {
    uint8_t addr = 0;
    uint8_t nob = 0;
    i2c_write_burst_blocking(I2C_PORT, FRAM_ADDR, &addr, 1);
    nob+=i2c_write_burst_blocking(I2C_PORT, FRAM_ADDR, buf, len - 1);
    nob+=i2c_write_blocking(I2C_PORT, FRAM_ADDR, buf + len - 1, 1, false);

    return nob;
}

uint8_t read_config_from_fram(struct config_block *cfg) {
    uint8_t addr = 0;
    i2c_write_burst_blocking(I2C_PORT, FRAM_ADDR, &addr, 1);
    return i2c_read_blocking(I2C_PORT, FRAM_ADDR, (uint8_t *)cfg, sizeof(config_block), false);
}

uint8_t retrieve_user(struct flat *user, struct config_block *cfg, uint16_t address) {
    uint8_t addr = sizeof(config_block) + (address - cfg->min_address) * sizeof(flat);
    i2c_write_burst_blocking(I2C_PORT, FRAM_ADDR, &addr, 1);
    return i2c_read_blocking(I2C_PORT, FRAM_ADDR, (uint8_t *)user, sizeof(flat), false);
}
