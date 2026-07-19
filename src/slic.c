//
// Created by Hans Gelke on 22.06.2026.
//
#include <zephyr/logging/log.h>
#include "gpio_i2c.h"
#include  "main.h"
#include "slic.h"

LOG_MODULE_REGISTER(slic, LOG_LEVEL_DBG);

int init_slic(void)
    {
        uint8_t mode = 0x00; //Direction of GPIO A Out
        uint8_t mask = 0xff;

        int ret3 = init_gpios();
        if (ret3 == 0)
        {
            LOG_INF("GPIO initialisation passed");
        }
    //
    // Initialize I2C
    //
        if (!device_is_ready(i2c_bus0))
        {
            LOG_ERR("I2C bus 0 not ready");
            return -ENODEV;
        }

        if (!device_is_ready(i2c_bus1))
        {
            LOG_ERR("I2C bus 1 not ready");
            return -ENODEV;
        }


        /* Initialization Values of GPIODIR and GPIO Registers */
        const uint8_t iodira[] = {0x00}; //Direction of pins 0=out, 1=in
        const uint8_t iodirb[] = {0xc0};
        const uint8_t init_gpioa[] = {0x08}; //Set SLIC to power savings, disable mux
        const uint8_t init_gpiob[] = {0x08};

        /*
         * Initialize IODIR Registers with values above
         */

        i2c_write_register(i2c_bus0,
                           PERIPH_ADDR_20,
                           IODIR_A, /* target register */
                           iodira,
                           sizeof(iodira));

        i2c_write_register(i2c_bus0,
                           PERIPH_ADDR_21,
                           IODIR_A, /* target register */
                           iodira,
                           sizeof(iodira));

        i2c_write_register(i2c_bus0,
                           PERIPH_ADDR_20,
                           IODIR_B, /* target register */
                           iodirb, sizeof(iodirb));

        i2c_write_register(i2c_bus0,
                           PERIPH_ADDR_21,
                           IODIR_B, /* target register */
                           iodirb, sizeof(iodirb));


        //-----------------------
        // Initialize GPIO 20 Register with values above
        //-------------------------
        i2c_write_register(i2c_bus0,
                           PERIPH_ADDR_20,
                           MCPREG_GPIO_A, /* target register */
                           init_gpioa,
                           sizeof(init_gpioa));


        i2c_write_register(i2c_bus0,
                           PERIPH_ADDR_20,
                           MCPREG_GPIO_B, /* target register */
                           init_gpiob,
                           sizeof(init_gpiob));

        //-----------------------
        // Initialize GPIO 21 Register with values above
        //-------------------------
        i2c_write_register(i2c_bus0,
                           PERIPH_ADDR_21,
                           MCPREG_GPIO_A, /* target register */
                           init_gpioa,
                           sizeof(init_gpioa));


        i2c_write_register(i2c_bus0,
                           PERIPH_ADDR_21,
                           MCPREG_GPIO_B, /* target register */
                           init_gpiob,
                           sizeof(init_gpiob));

        /* ----------------------------------------------------------------
             * READ I2C Register i2c 0x20
             * ---------------------------------------------------------------- */
        uint8_t rx_buf20[2] = {0};

        i2c_read_register(i2c_bus0,
                          PERIPH_ADDR_20,
                          0x12, /* register to read from */
                          rx_buf20,
                          sizeof(rx_buf20));


        /* ----------------------------------------------------------------
                 * READ I2C Register i2c 0x21
                 * ---------------------------------------------------------------- */
        uint8_t rx_buf21[2] = {0};

        i2c_read_register(i2c_bus0,
                          PERIPH_ADDR_21,
                          0x12, /* register to read from */
                          rx_buf21,
                          sizeof(rx_buf21));


        // ----------------------------------------------------------------
        // Set High Battery in SLIC 20*/
        //----------------------------------------------------------------
         mode = 0x00;
         mask = 0x10;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_20, //Is the subscriber number
                 MCPREG_GPIO_B,
                 mode,
                 mask);

        // ----------------------------------------------------------------
        // Set High Battery in SLIC 21*/
        //----------------------------------------------------------------
        mode = 0x00;
        mask = 0x10;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_21, //Is the subscriber number
                 MCPREG_GPIO_B,
                 mode,
                 mask);

        // ----------------------------------------------------------------
        // Sets Sets F0-F3 Bits in SLIC 20*/
        // ----------------------------------------------------------------
        mode = slic_fora;
        mask = slic_mode;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_20, //Is the subscriber number
                 MCPREG_GPIO_A,
                 mode,
                 mask);

        // ----------------------------------------------------------------
        // Sets Sets F0-F3 Bits in SLIC 21*/
        // ----------------------------------------------------------------
        set_slic(i2c_bus0,
                 PERIPH_ADDR_21, //Is the subscriber number
                 MCPREG_GPIO_A,
                 slic_fora,
                 slic_mode);


        // ----------------------------------------------------------------
        // Sets E0 Bit in SLIC 20*/
        //----------------------------------------------------------------
        mode = 0x80;
        mask = 0x80;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_20, //Is the subscriber number
                 MCPREG_GPIO_A,
                 mode,
                 mask);


        // ----------------------------------------------------------------
        // Sets E0 Bit in SLIC 21*/
        //----------------------------------------------------------------
        mode = 0x80;
        mask = 0x80;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_21, //Is the subscriber number
                 MCPREG_GPIO_A,
                 mode,
                 mask);


        // ----------------------------------------------------------------
        // Sets SWC_n Bit in SLIC 20*/
        //----------------------------------------------------------------
        mode = 0x20;
        mask = 0x20;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_20, //Is the subscriber number
                 MCPREG_GPIO_B,
                 mode,
                 mask);

        // ----------------------------------------------------------------
        // Sets SWC_n Bit in SLIC 21*/
        //----------------------------------------------------------------
        mode = 0x20;
        mask = 0x20;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_21, //Is the subscriber number
                 MCPREG_GPIO_B,
                 mode,
                 mask);


        // ----------------------------------------------------------------
        // Set Multiplexer 0x20 Phone 0
        //----------------------------------------------------------------
        //Receive (comes from matrix, goes to phone)
        mode = 0x01; // enabled, channel 0
        mask = 0x0f;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_20, //Is the subscriber number
                 MCPREG_GPIO_B,
                 mode,
                 mask);

        //Transmit (comes from phone, goes to matrix)
        mode = 0x00; // enabled, channel 0
        mask = 0x0f;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_20, //Is the subscriber number
                 MCPREG_GPIO_A,
                 mode,
                 mask);

        // ----------------------------------------------------------------
        // Set Multiplexer 0x21 Phone 1
        //----------------------------------------------------------------

        //Receive (comes from matrix, goes to phone)
        mode = 0x08; // enabled, channel 0
        mask = 0x0f;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_21, //Is the subscriber number
                 MCPREG_GPIO_B,
                 mode,
                 mask);

        //Transmit (comes from phone, goes to matrix)
        mode = 0x01; // enabled, channel 0
        mask = 0x0f;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_21, //Is the subscriber number
                 MCPREG_GPIO_A,
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
                          MCPREG_GPIO_A, /* register to read from */
                          rx_buf20,
                          sizeof(rx_buf20));

        LOG_INF("GPIO_A: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
                PERIPH_ADDR_20, MCPREG_GPIO_A, rx_buf20[0]);


        i2c_read_register(i2c_bus0,
                          PERIPH_ADDR_20,
                          MCPREG_GPIO_B, /* register to read from */
                          rx_buf20,
                          sizeof(rx_buf20));
        LOG_INF("MCPREG_GPIO_B: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
                PERIPH_ADDR_20, MCPREG_GPIO_B, rx_buf20[0]);


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
                          MCPREG_GPIO_A, /* register to read from */
                          rx_buf20,
                          sizeof(rx_buf20));

        LOG_INF("GPIO_A: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
                PERIPH_ADDR_21, MCPREG_GPIO_A, rx_buf20[0]);


        i2c_read_register(i2c_bus0,
                          PERIPH_ADDR_21,
                          MCPREG_GPIO_B, /* register to read from */
                          rx_buf20,
                          sizeof(rx_buf20));
        LOG_INF("MCPREG_GPIO_B: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
                PERIPH_ADDR_21, MCPREG_GPIO_B, rx_buf20[0]);





        return 0; // Return from main.c
    }



