//
// Created by Hans Gelke on 22.06.2026.
//
#include <zephyr/logging/log.h>
#include "gpio_i2c.h"

#include "slic.h"

LOG_MODULE_REGISTER(slic, LOG_LEVEL_DBG);

#define I2C_NODE_0    DT_NODELABEL(i2c0) //controls which bus is accessed
#define I2C_NODE_1    DT_NODELABEL(i2c1)
#define PERIPH_ADDR_20 DT_REG_ADDR(DT_NODELABEL(i2c0_peripheral_20))
#define PERIPH_ADDR_24 DT_REG_ADDR(DT_NODELABEL(i2c0_peripheral_24))
#define PERIPH_ADDR_21 DT_REG_ADDR(DT_NODELABEL(i2c1_peripheral_21))

#define GPIO_A 0x12 //Address of GPIO_A Register
#define GPIO_B 0x13
#define IODIR_A 0x00 //Address of direction Registers
#define IODIR_B 0x01

int init_slic(void)
    {
        int ret3 = init_gpios();
        if (ret3 == 0)
        {
            LOG_INF("GPIO initialisation passed");
        }
    //
    // Initialize I2C
    //
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
        const uint8_t iodira[] = {0x00}; //Direction of pins 0: out, 1= In
        const uint8_t iodirb[] = {0xc0};
        const uint8_t gpioa[] = {0x08}; //Set SLIC to power savings, disable mux
        const uint8_t gpiob[] = {0x08};

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
                           GPIO_A, /* target register */
                           gpioa,
                           sizeof(gpioa));


        i2c_write_register(i2c_bus0,
                           PERIPH_ADDR_20,
                           GPIO_B, /* target register */
                           gpiob,
                           sizeof(gpiob));

        //-----------------------
        // Initialize GPIO 21 Register with values above
        //-------------------------
        i2c_write_register(i2c_bus0,
                           PERIPH_ADDR_21,
                           GPIO_A, /* target register */
                           gpioa,
                           sizeof(gpioa));


        i2c_write_register(i2c_bus0,
                           PERIPH_ADDR_21,
                           GPIO_B, /* target register */
                           gpiob,
                           sizeof(gpiob));

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
        // Set controll register to output
        //----------------------------------------------------------------
        uint8_t mode = 0x00; //Direction of GPIO A Out
        uint8_t mask = 0xff;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_24,
                 IODIR_A,
                 mode,
                 mask);

         mode = 0x00; //Direction of GPIO B Out
         mask = 0xff;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_24,
                 IODIR_B,
                 mode,
                 mask);

         mode = 0x01;//Set Adress decoder to 1 for CMX no 1
         mask = 0x0f;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_24,
                 GPIO_B,
                 mode,
                 mask);


        // ----------------------------------------------------------------
        // Set High Battery in SLIC 20*/
        //----------------------------------------------------------------
         mode = 0x00;
         mask = 0x10;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_20, //Is the subscriber number
                 GPIO_B,
                 mode,
                 mask);

        // ----------------------------------------------------------------
        // Set High Battery in SLIC 21*/
        //----------------------------------------------------------------
        mode = 0x00;
        mask = 0x10;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_21, //Is the subscriber number
                 GPIO_B,
                 mode,
                 mask);

        // ----------------------------------------------------------------
        // Sets Sets F0-F3 Bits in SLIC 20*/
        // ----------------------------------------------------------------
        mode = slic_fora;
        mask = slic_mode;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_20, //Is the subscriber number
                 GPIO_A,
                 mode,
                 mask);

        // ----------------------------------------------------------------
        // Sets Sets F0-F3 Bits in SLIC 21*/
        // ----------------------------------------------------------------
        mode = slic_fora;
        mask = slic_mode;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_21, //Is the subscriber number
                 GPIO_A,
                 mode,
                 mask);


        // ----------------------------------------------------------------
        // Sets E0 Bit in SLIC 20*/
        //----------------------------------------------------------------
        mode = 0x00;
        mask = 0x80;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_20, //Is the subscriber number
                 GPIO_A,
                 mode,
                 mask);


        // ----------------------------------------------------------------
        // Sets E0 Bit in SLIC 21*/
        //----------------------------------------------------------------
        mode = 0x00;
        mask = 0x80;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_21, //Is the subscriber number
                 GPIO_A,
                 mode,
                 mask);


        // ----------------------------------------------------------------
        // Sets SWC_n Bit in SLIC 20*/
        //----------------------------------------------------------------
        mode = 0x20;
        mask = 0x20;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_20, //Is the subscriber number
                 GPIO_B,
                 mode,
                 mask);

        // ----------------------------------------------------------------
        // Sets SWC_n Bit in SLIC 21*/
        //----------------------------------------------------------------
        mode = 0x20;
        mask = 0x20;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_21, //Is the subscriber number
                 GPIO_B,
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
                 GPIO_B,
                 mode,
                 mask);

        //Transmit (comes from phone, goes to matrix)
        mode = 0x00; // enabled, channel 0
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
        mode = 0x08; // enabled, channel 0
        mask = 0x0f;
        set_slic(i2c_bus0,
                 PERIPH_ADDR_21, //Is the subscriber number
                 GPIO_B,
                 mode,
                 mask);

        //Transmit (comes from phone, goes to matrix)
        mode = 0x01; // enabled, channel 0
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

        //--------------------------------
        // Read registers 0x24
        //----------------------------------------------------------------
        i2c_read_register(i2c_bus0,
                          PERIPH_ADDR_24,
                          IODIR_A, /* register to read from */
                          rx_buf20,
                          sizeof(rx_buf20));

        LOG_INF("IODIR_A: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
                PERIPH_ADDR_24, IODIR_A, rx_buf20[0]);

        i2c_read_register(i2c_bus0,
                          PERIPH_ADDR_24,
                          IODIR_B, /* register to read from */
                          rx_buf20,
                          sizeof(rx_buf20));

        LOG_INF("IODIR_B: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
                PERIPH_ADDR_24, IODIR_B, rx_buf20[0]);

        //-------------------------------------------------------------------

        i2c_read_register(i2c_bus0,
                          PERIPH_ADDR_24,
                          GPIO_A, /* register to read from */
                          rx_buf20,
                          sizeof(rx_buf20));

        LOG_INF("GPIO_A: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
                PERIPH_ADDR_24, GPIO_A, rx_buf20[0]);


        i2c_read_register(i2c_bus0,
                          PERIPH_ADDR_24,
                          GPIO_B, /* register to read from */
                          rx_buf20,
                          sizeof(rx_buf20));
        LOG_INF("GPIO_B: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
                PERIPH_ADDR_24, GPIO_B, rx_buf20[0]);



        return 0; // Return from main.c
    }



