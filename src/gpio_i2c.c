#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <zephyr/sys/util.h>
#include <errno.h>
#include "gpio_i2c.h"

#define RST_N_PIN     6

#define GPIO_NODE DT_NODELABEL(gpio0)


LOG_MODULE_DECLARE(i2c_example, LOG_LEVEL_DBG);

const struct device* gpio_dev;

int init_gpios(void)
{
    int ret;
    gpio_dev = DEVICE_DT_GET(GPIO_NODE);
    ret = gpio_pin_configure(gpio_dev, RST_N_PIN, GPIO_OUTPUT_INACTIVE);
    gpio_pin_set(gpio_dev, RST_N_PIN, 1);

    //LOG_INF("Alle GPIOs initialisiert");
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

int init_gpios(void);

int set_slic(const struct device* bus,
             uint8_t dev_addr, //Is the subscriber number
             uint8_t* mode, //Is the mode to be set
             size_t len)
{
    uint8_t reg_addr = 0x12; //MCP23017 GPIOA Register address
    uint8_t* data = 0x0;
    int ret = i2c_write_read(bus,
                             dev_addr,
                             &reg_addr, sizeof(reg_addr), /* write phase */
                             data, len); /* read phase  */
    if (ret < 0)
    {
        LOG_ERR("SLIC read failed (addr=0x%02x reg=0x%02x): %d (%s)",
                dev_addr, reg_addr, ret, strerror(-ret));
    }

    //(byte & ~mask) | (mode & mask);
    return ret;
}
