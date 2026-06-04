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
#define GPIO_A 0x12 //Address of GPIO_A Register
#define GPIO_B 0x13
#define IODIR_A 0x00 //Address of direction Registers
#define IODIR_B 0x01


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

    /* Initialization Values of GPIODIR and GPIO Registers */
    const uint8_t iodira[] = {0x00};//Direction of pins 0: out, 1= In
    const uint8_t iodirb[] = {0xc0};
    const uint8_t gpioa[] = {0x08}; //Set SLIC to power savings, disable mux
    const uint8_t gpiob[] = {0x08};

    /*
     * Initialize IODIR Registers with values above
     */

    int ret1 = i2c_write_register(i2c_bus0,
                                  PERIPH_ADDR_20,
                                  IODIR_A, /* target register */
                                  iodira,
                                  sizeof(iodira));
    if (ret1 == 0)
    {
        LOG_INF("Write I2C-Register OK slave:0x%x, Reg_addr:0x%x, data:0x%x",
                PERIPH_ADDR_20, IODIR_A, iodira[0]);
    }

    int ret21 = i2c_write_register(i2c_bus0,
                                      PERIPH_ADDR_21,
                                      IODIR_A, /* target register */
                                      iodira,
                                      sizeof(iodira));
    if (ret21 == 0)
    {
        LOG_INF("Write I2C-Register OK slave:0x%x, Reg_addr:0x%x, data:0x%x",
                PERIPH_ADDR_21, IODIR_A, iodira[0]);
    }

    int ret11 = i2c_write_register(i2c_bus0,
                                     PERIPH_ADDR_20,
                                     IODIR_B, /* target register */
                                     iodirb,sizeof(iodirb));
    if (ret11 == 0)
    {
        LOG_INF("Write I2C-Register OK slave:0x%x, Reg_addr:0x%x, data:0x%x",
                PERIPH_ADDR_20, IODIR_B, iodirb[0]);
    }
    int ret31 = i2c_write_register(i2c_bus0,
                                         PERIPH_ADDR_21,
                                         IODIR_B, /* target register */
                                         iodirb,sizeof(iodirb));
    if (ret31 == 0)
    {
        LOG_INF("Write I2C-Register OK slave:0x%x, Reg_addr:0x%x, data:0x%x",
                PERIPH_ADDR_21, IODIR_B, iodirb[0]);
    }

//-----------------------
// Initialize GPIO 20 Register with values above
//-------------------------
    int ret2 = i2c_write_register(i2c_bus0,
                                  PERIPH_ADDR_20,
                                  GPIO_A, /* target register */
                                  gpioa,
                                  sizeof(gpioa));
    if (ret2 == 0)
    {
        LOG_INF("Write I2C-Register OK slave:0x%x, data:0x%x",
                PERIPH_ADDR_20, gpioa[0]);
    }


    int ret12 = i2c_write_register(i2c_bus0,
                                      PERIPH_ADDR_20,
                                      GPIO_B, /* target register */
                                      gpiob,
                                      sizeof(gpiob));
    if (ret12 == 0)
    {
        LOG_INF("Write I2C-Register OK slave:0x%x, data:0x%x",
                PERIPH_ADDR_20, gpiob[0]);
    }

    //-----------------------
    // Initialize GPIO 21 Register with values above
    //-------------------------
    int ret32 = i2c_write_register(i2c_bus0,
                                  PERIPH_ADDR_21,
                                  GPIO_A, /* target register */
                                  gpioa,
                                  sizeof(gpioa));
    if (ret32 == 0)
    {
        LOG_INF("Write I2C-Register OK slave:0x%x, data:0x%x",
                PERIPH_ADDR_21, gpioa[0]);
    }


    int ret33 = i2c_write_register(i2c_bus0,
                                      PERIPH_ADDR_21,
                                      GPIO_B, /* target register */
                                      gpiob,
                                      sizeof(gpiob));
    if (ret33 == 0)
    {
        LOG_INF("Write I2C-Register OK slave:0x%x, data:0x%x",
                PERIPH_ADDR_21, gpiob[0]);
    }
    /* ----------------------------------------------------------------
         * READ I2C Register i2c 0x20
         * ---------------------------------------------------------------- */
    uint8_t rx_buf20[2] = {0};

    int ret = i2c_read_register(i2c_bus0,
                                PERIPH_ADDR_20,
                                0x12, /* register to read from */
                                rx_buf20,
                                sizeof(rx_buf20));

    if (ret == 0)
    {
        LOG_INF("Read OK  -> slave 0x%02x, data: 0x%02x",
                PERIPH_ADDR_20, rx_buf20[0]);
    }
    /* ----------------------------------------------------------------
             * READ I2C Register i2c 0x21
             * ---------------------------------------------------------------- */
    uint8_t rx_buf21[2] = {0};

    int ret41 = i2c_read_register(i2c_bus0,
                                PERIPH_ADDR_21,
                                0x12, /* register to read from */
                                rx_buf21,
                                sizeof(rx_buf21));

    if (ret41 == 0)
    {
        LOG_INF("Read OK  -> slave 0x%02x, data: 0x%02x",
                PERIPH_ADDR_21, rx_buf21[0]);
    }

    // ----------------------------------------------------------------
    // Set High Battery in SLIC 20*/
    //----------------------------------------------------------------
    uint8_t mode =0x00;
    uint8_t mask =0x10;
    int ret9 = set_slic(i2c_bus0,
                        PERIPH_ADDR_20, //Is the subscriber number
                        GPIO_B,
                        mode,
                        mask);
    if (ret9 == 0)
    {
        LOG_INF("SLIC OK slave:0x%x, mode:0x%x, mask:0x%x",
                PERIPH_ADDR_20, mode, mask);
    }
    // ----------------------------------------------------------------
    // Set High Battery in SLIC 21*/
    //----------------------------------------------------------------
    mode =0x00;
    mask =0x10;
    int ret29 = set_slic(i2c_bus0,
                        PERIPH_ADDR_21, //Is the subscriber number
                        GPIO_B,
                        mode,
                        mask);
    if (ret29 == 0)
    {
        LOG_INF("SLIC OK slave:0x%x, mode:0x%x, mask:0x%x",
                PERIPH_ADDR_21, mode, mask);
    }

    // ----------------------------------------------------------------
    // Sets Sets F0-F3 Bits in SLIC 20*/
    // ----------------------------------------------------------------
    mode =slic_fora;
    mask =slic_mode;
    int ret4 = set_slic(i2c_bus0,
                        PERIPH_ADDR_20, //Is the subscriber number
                        GPIO_A,
                        mode,
                        mask);
    if (ret4 == 0)
    {
        LOG_INF("SLIC OK slave:0x%x, mode:0x%x, mask:0x%x",
                PERIPH_ADDR_20, mode, mask);
    }
    // ----------------------------------------------------------------
    // Sets Sets F0-F3 Bits in SLIC 21*/
    // ----------------------------------------------------------------
    mode =slic_fora;
    mask =slic_mode;
    int ret44 = set_slic(i2c_bus0,
                        PERIPH_ADDR_21, //Is the subscriber number
                        GPIO_A,
                        mode,
                        mask);
    if (ret44 == 0)
    {
        LOG_INF("SLIC OK slave:0x%x, mode:0x%x, mask:0x%x",
                PERIPH_ADDR_21, mode, mask);
    }

    // ----------------------------------------------------------------
    // Sets E0 Bit in SLIC 20*/
     //----------------------------------------------------------------
    mode =0x00;
    mask =0x80;
    int ret8 = set_slic(i2c_bus0,
                        PERIPH_ADDR_20, //Is the subscriber number
                        GPIO_A,
                        mode,
                        mask);
    if (ret8 == 0)
    {
        LOG_INF("SLIC OK slave:0x%x, mode:0x%x, mask:0x%x",
                PERIPH_ADDR_20, mode, mask);
    }

    // ----------------------------------------------------------------
    // Sets E0 Bit in SLIC 21*/
    //----------------------------------------------------------------
    mode =0x00;
    mask =0x80;
    int ret58 = set_slic(i2c_bus0,
                        PERIPH_ADDR_21, //Is the subscriber number
                        GPIO_A,
                        mode,
                        mask);
    if (ret58 == 0)
    {
        LOG_INF("SLIC OK slave:0x%x, mode:0x%x, mask:0x%x",
                PERIPH_ADDR_21, mode, mask);
    }

    // ----------------------------------------------------------------
    // Sets SWC_n Bit in SLIC 20*/
    //----------------------------------------------------------------
    mode =0x20;
    mask =0x20;
    int ret13 = set_slic(i2c_bus0,
                        PERIPH_ADDR_20, //Is the subscriber number
                        GPIO_B,
                        mode,
                        mask);
    if (ret13 == 0)
    {
        LOG_INF("SLIC OK slave:0x%x, mode:0x%x, mask:0x%x",
                PERIPH_ADDR_20, mode, mask);
    }
    // ----------------------------------------------------------------
    // Sets SWC_n Bit in SLIC 21*/
    //----------------------------------------------------------------
    mode =0x20;
    mask =0x20;
    int ret53 = set_slic(i2c_bus0,
                        PERIPH_ADDR_21, //Is the subscriber number
                        GPIO_B,
                        mode,
                        mask);
    if (ret53 == 0)
    {
        LOG_INF("SLIC OK slave:0x%x, mode:0x%x, mask:0x%x",
                PERIPH_ADDR_21, mode, mask);
    }

    // ----------------------------------------------------------------
    // Set Multiplexer 0x20 Phone 0
    //----------------------------------------------------------------
    //Receive (comes from matrix, goes to phone)
    mode = 0x00; // enabled, channel 0
    mask = 0x0f;
    set_slic(i2c_bus0,
             PERIPH_ADDR_20, //Is the subscriber number
             GPIO_B,
             mode,
             mask);

    //Transmit (comes from phone, goes to matrix)
    mode = 0x01; // enabled, channel 0
    mask = 0x0f;
    set_slic(i2c_bus0,
             PERIPH_ADDR_20, //Is the subscriber number
             GPIO_A,
             mode,
             mask);

    // ----------------------------------------------------------------
    // Set Multiplexer 0x21 Phone 1
    //----------------------------------------------------------------

    //Receive (comes from matrix, goes to phone)
    mode = 0x01; // enabled, channel 0
    mask = 0x0f;
    set_slic(i2c_bus0,
             PERIPH_ADDR_21, //Is the subscriber number
             GPIO_B,
             mode,
             mask);

    //Transmit (comes from phone, goes to matrix)
    mode = 0x00; // enabled, channel 0
    mask = 0x0f;
    set_slic(i2c_bus0,
             PERIPH_ADDR_21, //Is the subscriber number
             GPIO_A,
             mode,
             mask);


    // ----------------------------------------------------------------
    // Read registers 0x20
    //----------------------------------------------------------------
    i2c_read_register(i2c_bus0,
                                        PERIPH_ADDR_20,
                                        IODIR_A, /* register to read from */
                                        rx_buf20,
                                        sizeof(rx_buf20));

    LOG_INF("IODIR_A: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
            PERIPH_ADDR_20, IODIR_A, rx_buf20[0]);

    i2c_read_register(i2c_bus0,
                                            PERIPH_ADDR_20,
                                            IODIR_B, /* register to read from */
                                            rx_buf20,
                                            sizeof(rx_buf20));

    LOG_INF("IODIR_B: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
            PERIPH_ADDR_20, IODIR_B, rx_buf20[0]);

//-------------------------------------------------------------------

    i2c_read_register(i2c_bus0,
                                    PERIPH_ADDR_20,
                                    GPIO_A, /* register to read from */
                                    rx_buf20,
                                    sizeof(rx_buf20));

        LOG_INF("GPIO_A: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
                PERIPH_ADDR_20, GPIO_A, rx_buf20[0]);



    i2c_read_register(i2c_bus0,
                                        PERIPH_ADDR_20,
                                        GPIO_B, /* register to read from */
                                        rx_buf20,
                                        sizeof(rx_buf20));
    LOG_INF("GPIO_B: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
            PERIPH_ADDR_20, GPIO_B, rx_buf20[0]);


    // ----------------------------------------------------------------
    // Read registers 0x21
    //----------------------------------------------------------------
    i2c_read_register(i2c_bus0,
                                        PERIPH_ADDR_21,
                                        IODIR_A, /* register to read from */
                                        rx_buf20,
                                        sizeof(rx_buf20));

    LOG_INF("IODIR_A: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
            PERIPH_ADDR_21, IODIR_A, rx_buf20[0]);

    i2c_read_register(i2c_bus0,
                                            PERIPH_ADDR_21,
                                            IODIR_B, /* register to read from */
                                            rx_buf20,
                                            sizeof(rx_buf20));

    LOG_INF("IODIR_B: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
            PERIPH_ADDR_21, IODIR_B, rx_buf20[0]);

    //-------------------------------------------------------------------

    i2c_read_register(i2c_bus0,
                                    PERIPH_ADDR_21,
                                    GPIO_A, /* register to read from */
                                    rx_buf20,
                                    sizeof(rx_buf20));

    LOG_INF("GPIO_A: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
            PERIPH_ADDR_21, GPIO_A, rx_buf20[0]);



    i2c_read_register(i2c_bus0,
                                        PERIPH_ADDR_21,
                                        GPIO_B, /* register to read from */
                                        rx_buf20,
                                        sizeof(rx_buf20));
    LOG_INF("GPIO_B: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
            PERIPH_ADDR_21, GPIO_B, rx_buf20[0]);

    return 0; // Return from main.c
}
