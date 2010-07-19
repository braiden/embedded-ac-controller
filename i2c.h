#pragma once

#ifndef IC2_CLK
#define I2C_CLK 5000UL
#endif

void i2c_init();
uint8_t i2c_start();
void i2c_stop();
uint8_t i2c_write(uint8_t addr, uint8_t *buffer, uint8_t len);
uint8_t i2c_read(uint8_t addr, uint8_t *buffer, uint8_t len);
