#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
//#include <zephyr/sys/util.h>
//#include <errno.h>
#include "gpio_i2c.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define RST_N_PIN     6

#define GPIO_NODE DT_NODELABEL(gpio0)


LOG_MODULE_REGISTER(gpio_i2c, LOG_LEVEL_NONE);

const struct device* gpio_dev;
const struct device *i2c_bus0 = DEVICE_DT_GET(I2C_NODE_0);
const struct device *i2c_bus1 = DEVICE_DT_GET(I2C_NODE_1);

int init_gpios(void)
{
    int ret;
    gpio_dev = DEVICE_DT_GET(GPIO_NODE);
    ret = gpio_pin_configure(gpio_dev, RST_N_PIN, GPIO_OUTPUT_INACTIVE);
    gpio_pin_set(gpio_dev, RST_N_PIN, 1);

    LOG_INF("I2C RESET PIN RELEASED");
    return 0;
}


int i2c_write_register(const struct device* bus,
                       uint8_t dev_addr,
                       uint8_t reg_addr,
                       const uint8_t* data,
                       size_t len)
{
    /* Build one contiguous buffer: [reg_addr | data...]
     * The RP2350 I2C controller sends these as a single
     * transaction without a repeated START in between.   */
    uint8_t buf[1 + len]; /* VLA; fine for small len */
    buf[0] = reg_addr;
    memcpy(&buf[1], data, len);

    int ret = i2c_write(bus, buf, sizeof(buf), dev_addr);
    if (ret < 0)
    {
        LOG_ERR("I2C write failed (addr=0x%02x reg=0x%02x): %d (%s)",
                dev_addr, reg_addr, ret, strerror(-ret));
        LOG_ERR("2 I2C write failed 2: %d (%s)", ret, strerror(-ret));
    }
    return ret;
}

int i2c_read_register(const struct device* bus,
                      uint8_t dev_addr,
                      uint8_t reg_addr,
                      uint8_t* data,
                      size_t len)
{
    /* Write the register address first, then read back the data.
     * i2c_write_read() issues a repeated START between the two
     * phases — no STOP in between — which most peripherals require. */
    int ret = i2c_write_read(bus,
                             dev_addr,
                             &reg_addr, sizeof(reg_addr), /* write phase */
                             data, len); /* read phase  */
    if (ret < 0)
    {
        LOG_ERR("I2C read failed (addr=0x%02x reg=0x%02x): %d (%s)",
                dev_addr, reg_addr, ret, strerror(-ret));
    }
    return ret;
}



