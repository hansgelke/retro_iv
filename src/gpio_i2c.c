#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <zephyr/sys/util.h>
#include <errno.h>
#include "gpio_i2c.h"

LOG_MODULE_DECLARE(i2c_example, LOG_LEVEL_DBG);

/* Grab the I2C bus node and the peripheral's 7-bit address
 * from the device tree.  Change the alias/label to match
 * whatever you named your node in the overlay.             */
//#define I2C_NODE      DT_NODELABEL(i2c0)
//#define PERIPH_ADDR   DT_REG_ADDR(DT_NODELABEL(my_peripheral))

/* ------------------------------------------------------------------ *
 * Generic write helper                                                *
 *                                                                     *
 * bus      '96 I2C bus device                                           *
 * dev_addr '96 7-bit slave address                                      *
 * reg_addr '96 register / command byte inside the peripheral           *
 * data     '96 payload bytes to write after the register byte          *
 * len      '96 number of payload bytes                                  *
 * ------------------------------------------------------------------ */

int i2c_write_register(const struct device *bus,
                               uint8_t dev_addr,
                               uint8_t reg_addr,
                               const uint8_t *data,
                               size_t len)
{
    /* Build one contiguous buffer: [reg_addr | data...]
     * The RP2350 I2C controller sends these as a single
     * transaction without a repeated START in between.   */
    uint8_t buf[1 + len];          /* VLA; fine for small len */
    buf[0] = reg_addr;
    memcpy(&buf[1], data, len);

    int ret = i2c_write(bus, buf, sizeof(buf), dev_addr);
    if (ret < 0) {
        LOG_ERR("I2C write failed (addr=0x%02x reg=0x%02x): %d (%s)",
        dev_addr, reg_addr, ret, strerror(-ret));
        LOG_ERR("2 I2C write failed 2: %d (%s)", ret, strerror(-ret));
    }
    return ret;
}
int i2c_read_register(const struct device *bus,
                              uint8_t dev_addr,
                              uint8_t reg_addr,
                              uint8_t *data,
                              size_t len)
{
    /* Write the register address first, then read back the data.
     * i2c_write_read() issues a repeated START between the two
     * phases — no STOP in between — which most peripherals require. */
    int ret = i2c_write_read(bus,
                             dev_addr,
                             &reg_addr, sizeof(reg_addr),  /* write phase */
                             data,      len);              /* read phase  */
    if (ret < 0) {
        LOG_ERR("I2C read failed (addr=0x%02x reg=0x%02x): %d (%s)",
                dev_addr, reg_addr, ret, strerror(-ret));
    }
    return ret;
}