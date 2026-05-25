#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>

#include "gpio_i2c.h"
#include "main.h"

LOG_MODULE_REGISTER(i2c_example, LOG_LEVEL_DBG);

#define I2C_NODE_0    DT_NODELABEL(i2c0) //controls which bus is accessed
#define I2C_NODE_1    DT_NODELABEL(i2c1)
#define PERIPH_ADDR_20 DT_REG_ADDR(DT_NODELABEL(my_peripheral_20))
#define PERIPH_ADDR_21 DT_REG_ADDR(DT_NODELABEL(my_peripheral_21))
#define PERIPH_ADDR_23 DT_REG_ADDR(DT_NODELABEL(my_peripheral_23))

/* Grab the I2C bus node and the peripheral's 7-bit address
 * from the device tree.  Change the alias/label to match
 * whatever you named your node in the overlay.             */

int main(void)
{
    int ret3 = init_gpios();
    if (ret3 == 0)
    {
        LOG_INF("GPIO initialisation passed");
    }

    const struct device* i2c_bus0 = DEVICE_DT_GET(I2C_NODE_0);

    if (!device_is_ready(i2c_bus0))
    {
        LOG_ERR("I2C bus 0 not ready");
        return -ENODEV;
    }

    const struct device* i2c_bus1 = DEVICE_DT_GET(I2C_NODE_1);

    if (!device_is_ready(i2c_bus1))
    {
        LOG_ERR("I2C bus 1 not ready");
        return -ENODEV;
    }

    /* Example: write two bytes to register 0x01 of the peripheral */

    const uint8_t iodira[] = {0x00};
    const uint8_t gpioa[] = {0xaa};

    /*
     *
     *
     * Write to peripheral 0
     *
     *
     */

    int ret1 = i2c_write_register(i2c_bus0,
                                  PERIPH_ADDR_20,
                                  0x00, /* target register */
                                  iodira,
                                  sizeof(iodira));
    if (ret1 == 0)
    {
        LOG_INF("Write OK payload 0 slave 0x%02x, reg ?, "
                "data: 0x%02x",
                PERIPH_ADDR_20, iodira[0]);
    }

    int ret2 = i2c_write_register(i2c_bus0,
                                  PERIPH_ADDR_20,
                                  0x12, /* target register */
                                  gpioa,
                                  sizeof(gpioa));

    if (ret2 == 0)
    {
        LOG_INF("Write OK payload 1  slave 0x%02x, reg ?, "
                "data: 0x%02x",
                PERIPH_ADDR_20, gpioa[0]);
    }
    /* ----------------------------------------------------------------
         * READ example
         * Read 2 bytes back from register 0x01 of the peripheral
         * ---------------------------------------------------------------- */
    uint8_t rx_buf[2] = {0};

    int ret = i2c_read_register(i2c_bus0,
                                PERIPH_ADDR_20,
                                0x12, /* register to read from */
                                rx_buf,
                                sizeof(rx_buf));

    if (ret == 0)
    {
        LOG_INF("Read OK  -> slave 0x%02x, reg ?, "
                "data: 0x%02x 0x%02x",
                PERIPH_ADDR_20, rx_buf[1], rx_buf[0]);
    }

    int ret4 = set_slic(i2c_bus0,
                        PERIPH_ADDR_20, //Is the subscriber number
                        rx_buf,
                        sizeof(rx_buf));
    if (ret4 == 0)
    {
        LOG_INF("SLIC OK  -> slave 0x%02x, "
                "data: 0x%02x 0x%02x",
                PERIPH_ADDR_20, rx_buf[1], rx_buf[0]);
    }
    return 0; // Return from main.c
}
