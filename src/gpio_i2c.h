//
// Created by Hans Gelke on 22.04.2026.
//

#ifndef SRC_GPIO_I2C_H
#define SRC_GPIO_I2C_H

#define slic_lpsb (0x00)
#define slic_fora (0x10)
#define slic_urng (0x20)
#define slic_reva (0x30)
#define slic_ring (0x40)
#define slic_loop (0x50)
#define slic_tipo (0x60)
#define slic_powd (0x70)
#define slic_mode (0x70)

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
                    uint8_t dev_addr,
                    uint8_t reg_addr,
                    uint8_t mode,//Is the subscriber number
                    uint8_t mask);  //Is the mode to be set


#endif //SRC_GPIO_I2C_H