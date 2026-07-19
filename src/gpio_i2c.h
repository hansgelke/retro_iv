//
// Created by Hans Gelke on 22.04.2026.
//

#ifndef SRC_GPIO_I2C_H
#define SRC_GPIO_I2C_H
#include <zephyr/device.h>

/* ------------------------------------------------------------------ */
/* I2C bus node labels — devicetree references                        */
/* ------------------------------------------------------------------ */
#define I2C_NODE_0      DT_NODELABEL(i2c0)
#define I2C_NODE_1      DT_NODELABEL(i2c1)

/* ------------------------------------------------------------------ */
/* Shared I2C bus handles — defined in gpio_i2c.c                     */
/* ------------------------------------------------------------------ */
extern const struct device *i2c_bus0;
extern const struct device *i2c_bus1;

/* ------------------------------------------------------------------ */
/* I2C peripheral addresses — derived from devicetree                 */
/* ------------------------------------------------------------------ */
#define PERIPH_ADDR_20  DT_REG_ADDR(DT_NODELABEL(i2c0_peripheral_20))
#define PERIPH_ADDR_21  DT_REG_ADDR(DT_NODELABEL(i2c0_peripheral_21))
#define PERIPH_ADDR_24  DT_REG_ADDR(DT_NODELABEL(i2c0_peripheral_24))

/* ------------------------------------------------------------------ */
/* MCP23017 register addresses                                         */
/* ------------------------------------------------------------------ */
#define IODIR_A         0x00    /* I/O direction register A           */
#define IODIR_B         0x01    /* I/O direction register B           */
#define MCPREG_IOPOL_A          0x02    /* GPIO port A register               */
#define MCPREG_IOPOL_B          0x03    /* GPIO port B register */
#define MCPREG_GPINTEN_A          0x04    /* GPIO port B register               */
#define MCPREG_GPINTEN_B          0x05    /* GPIO port B register               */
#define MCPREG_DEFVAL_A          0x06    /* GPIO port B register               */
#define MCPREG_DEFVAL_B          0x07    /* GPIO port B register               */
#define MCPREG_INTCON_A          0x08    /* GPIO port B register               */
#define MCPREG_INTCON_B          0x09    /* GPIO port B register               */
#define MCPREG_IOCON            0x0a    /* GPIO port B register               */
#define MCPREG_GPPU_A          0x0c    /* GPIO port B register               */
#define MCPREG_GPPU_B          0x0d    /* GPIO port B register               */
#define MCPREG_INTF_A          0x0e    /* GPIO port B register               */
#define MCPREG_INTF_B          0x0f    /* GPIO port B register               */
#define MCPREG_INTCAP_A          0x10    /* GPIO port B register               */
#define MCPREG_INTCAP_B          0x11    /* GPIO port B register               */
#define MCPREG_GPIO_A          0x12    /* GPIO port B register               */
#define MCPREG_GPIO_B          0x13    /* GPIO port B register               */
#define MCPREG_OLAT_A          0x14    /* GPIO port B register               */
#define MCPREG_OLAT_B          0x15    /* GPIO port B register               */

#define slic_lpsb (0x00) // Low Power Standby
#define slic_fora (0x10) // Forward active
#define slic_urng (0x20) // unballanced ringing
#define slic_reva (0x30)//Reverse active
#define slic_ring (0x40)//Ringing
#define slic_loop (0x50)// forward loop
#define slic_tipo (0x60) //Tip Open
#define slic_powd (0x70) //Power denial
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