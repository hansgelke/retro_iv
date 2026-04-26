//
// Created by Hans Gelke on 22.04.2026.
//

#ifndef SRC_GPIO_I2C_H
#define SRC_GPIO_I2C_H


#include <zephyr/device.h>
#include <stdint.h>
#include <stddef.h>


int i2c_write_register(const struct device *bus,
                               uint8_t dev_addr,
                               uint8_t reg_addr,
                               const uint8_t *data,
                               size_t len);

int i2c_read_register(const struct device *bus,
                              uint8_t dev_addr,
                              uint8_t reg_addr,
                              uint8_t *data,
                              size_t len);

#endif //SRC_GPIO_I2C_H