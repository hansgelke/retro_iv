//
// Created by Hans Gelke on 22.04.2026.
//

#ifndef SRC_GPIO_I2C_H
#define SRC_GPIO_I2C_H


#include <zephyr/device.h>
#include <stdint.h>
#include <stddef.h>
#define slic_lpsb (0x00)
#define slic_fora (0x01)
#define slic_urng (0x02)
#define slic_reva (0x03)
#define slic_ring (0x04)
#define slic_loop (0x05)
#define slic_tipo (0x06)
#define slic_powd (0x07)
#define slic_mask (0x70)

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
int init_gpios(void);

int set_slic(const struct device *bus,
                    uint8_t dev_addr, //Is the subscriber number
                    uint8_t *mode,  //Is the mode to be set
                    size_t len);



#endif //SRC_GPIO_I2C_H