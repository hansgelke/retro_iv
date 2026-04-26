#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <errno.h>
#include "gpio_i2c.h"

LOG_MODULE_REGISTER(i2c_example, LOG_LEVEL_DBG);

#define I2C_NODE    DT_NODELABEL(i2c0)
#define PERIPH_ADDR DT_REG_ADDR(DT_NODELABEL(my_peripheral))

/* Grab the I2C bus node and the peripheral's 7-bit address
 * from the device tree.  Change the alias/label to match
 * whatever you named your node in the overlay.             */

int main(void)
{
    const struct device *i2c_bus = DEVICE_DT_GET(I2C_NODE);

    if (!device_is_ready(i2c_bus)) {
        LOG_ERR("I2C bus not ready");
        return -ENODEV;
    }

    /* Example: write two bytes to register 0x01 of the peripheral */
    const uint8_t payload0[] = { 0xfe };
    const uint8_t payload1[] = { 0x01 };


    int ret1 = i2c_write_register(i2c_bus,
                                  PERIPH_ADDR,
                                  0x00,          /* target register */
                                  payload0,
                                  sizeof(payload0));
    if (ret1 == 0) {
        LOG_INF("Write OK payload 0 slave 0x%02x, reg ?, "
                "data: 0x%02x",
                PERIPH_ADDR, payload0[0]);
    }

    int ret2 = i2c_write_register(i2c_bus,
                                      PERIPH_ADDR,
                                      0x09,          /* target register */
                                      payload1,
                                      sizeof(payload1));

    if (ret2 == 0) {
        LOG_INF("Write OK payload 1  slave 0x%02x, reg ?, "
                "data: 0x%02x",
                PERIPH_ADDR, payload1[0]);
    }



/* ----------------------------------------------------------------
     * READ example
     * Read 2 bytes back from register 0x01 of the peripheral
     * ---------------------------------------------------------------- */
uint8_t rx_buf[2] = { 0 };

int ret = i2c_read_register(i2c_bus,
                         PERIPH_ADDR,
                         0x09,          /* register to read from */
                         rx_buf,
                         sizeof(rx_buf));

if (ret == 0) {
    LOG_INF("Read OK  -> slave 0x%02x, reg ?, "
            "data: 0x%02x 0x%02x",
            PERIPH_ADDR, rx_buf[0], rx_buf[1]);
}

return 0;
}