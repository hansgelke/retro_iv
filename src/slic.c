//
// Created by Hans Gelke on 22.06.2026.
//
#include <zephyr/logging/log.h>
#include "gpio_i2c.h"
#include  "main.h"
#include "slic.h"
#include <zephyr/drivers/i2c.h>

LOG_MODULE_REGISTER(slic, LOG_LEVEL_DBG);

//----------------------------------------
// Initialice I2C in SLICs
//-----------------------------------------

int init_slic(void)
    {
        uint8_t mode = 0x00; //Direction of GPIO A Out
        uint8_t mask = 0xff;

        int ret3 = init_gpios(); //Removes I2C chips reset line
        if (ret3 == 0)
        {
            LOG_INF("GPIO initialisation passed");
        }
    //----------------------------------------
    // Check if I2C is ready
    //-----------------------------------------
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

        const uint8_t init_gpioa[] = {0x08}; //Set SLIC to power savings, disable mux
        const uint8_t init_gpiob[] = {0x08};

    // ----------------------------------------------------------------
// Initialize SLIC IODIR Registers
//---------------------------------------------------------------

    set_slic(i2c_bus0,PERIPH_ADDR_20, MCPREG_IODIR_B,SLIC_DIR_B,SLIC_DIR);

    set_slic(i2c_bus0,PERIPH_ADDR_21, MCPREG_IODIR_B,SLIC_DIR_B,SLIC_DIR);

    set_slic(i2c_bus0,PERIPH_ADDR_20, MCPREG_IODIR_A,SLIC_DIR_A,SLIC_DIR);

    set_slic(i2c_bus0,PERIPH_ADDR_21, MCPREG_IODIR_A,SLIC_DIR_A,SLIC_DIR);

        //-----------------------
        // Initialize SLIC TX MUXER  with default values
        //-------------------------
    set_slic_txmux(i2c_bus0,0, SLIC_MUX_DIS);
    set_slic_txmux(i2c_bus0,1, SLIC_MUX_DIS);

    //-----------------------
    // Initialize SLIC RX MUXER  with default values
    //-------------------------
    set_slic_rxmux(i2c_bus0,0, SLIC_MUX_DIS);
    set_slic_rxmux(i2c_bus0,1, SLIC_MUX_DIS);

        // ----------------------------------------------------------------
        // Set Low/HIGH Battery in SLIC 20*/
        //----------------------------------------------------------------
        set_slic(i2c_bus0, PERIPH_ADDR_20, MCPREG_GPIO_B,BATHI,BATSEL);
    set_slic(i2c_bus0,PERIPH_ADDR_21, MCPREG_GPIO_B,BATHI,BATSEL);

        // ----------------------------------------------------------------
        // Sets Sets F0-F3 Bits in SLIC 20 and 21
        // ----------------------------------------------------------------
        set_slic_mode(i2c_bus0, 1, SLIC_LPSB);
        set_slic_mode(i2c_bus0, 0, SLIC_LPSB);

        // ----------------------------------------------------------------
        // Sets E0 Bit in SLICs
        //----------------------------------------------------------------

        set_slic(i2c_bus0,PERIPH_ADDR_20, MCPREG_GPIO_A,SLIC_E0,SLIC_E0_MASK);
        set_slic(i2c_bus0,PERIPH_ADDR_21, MCPREG_GPIO_A,SLIC_E0,SLIC_E0_MASK);


        // ----------------------------------------------------------------
        // Sets SWC_n Bit in SLICs
        //----------------------------------------------------------------

        set_slic(i2c_bus0,PERIPH_ADDR_20, MCPREG_GPIO_B,SWC_N,SWC_N_MASK);
        set_slic(i2c_bus0,PERIPH_ADDR_21, MCPREG_GPIO_B,SWC_N,SWC_N_MASK);



        // ----------------------------------------------------------------
        // Set Multiplexer 0x20 Phone 0 BIS HIERHER MARKER
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
        // Read IODIR A and B Register for checking only
        //----------------------------------------------------------------
    uint8_t rx_buf20[2] = {0};
        i2c_read_register(i2c_bus0,
                          PERIPH_ADDR_20,
                          MCPREG_IODIR_A, /* register to read from */
                          rx_buf20,
                          sizeof(rx_buf20));

        LOG_INF("IODIR_A: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
                PERIPH_ADDR_20, MCPREG_IODIR_A, rx_buf20[0]);

        i2c_read_register(i2c_bus0,
                          PERIPH_ADDR_20,
                          MCPREG_IODIR_B, /* register to read from */
                          rx_buf20,
                          sizeof(rx_buf20));

        LOG_INF("IODIR_B: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
                PERIPH_ADDR_20, MCPREG_IODIR_B, rx_buf20[0]);

    //-------------------------------------------------------------------
    // Read GPIO A and B Register for checking only
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
        // Read IODIR A and B Register for checking only
        //----------------------------------------------------------------
        i2c_read_register(i2c_bus0,
                          PERIPH_ADDR_21,
                          MCPREG_IODIR_A, /* register to read from */
                          rx_buf20,
                          sizeof(rx_buf20));

        LOG_INF("IODIR_A: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
                PERIPH_ADDR_21, MCPREG_IODIR_A, rx_buf20[0]);

        i2c_read_register(i2c_bus0,
                          PERIPH_ADDR_21,
                          MCPREG_IODIR_B, /* register to read from */
                          rx_buf20,
                          sizeof(rx_buf20));

        LOG_INF("IODIR_B: slave 0x%02x, reg_addr: 0x%02x data: 0x%02x",
                PERIPH_ADDR_21, MCPREG_IODIR_B, rx_buf20[0]);

        //-------------------------------------------------------------------
        // Read GPIO A and B Register for checking only
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

/* ----------------------------------------------------------------*/
/* Function Sets Mode Register in SLIC  */
/* -------------------------------------------------------------*/

int set_slic_mode(const struct device* bus,
             uint8_t device, //Is the subscriber number
             uint8_t mode)
{
    uint8_t reg_addr = 0x12; //GPIO A
    uint8_t mode_mask = 0x70; //Bits 4-6
    uint8_t byte[2] = {0x0};
    uint8_t dev_addr;

    switch (device)
    {
    case 0x0:
        dev_addr = PERIPH_ADDR_20;
        break;
    case 0x1:
        dev_addr = PERIPH_ADDR_21;
        break;
    default:
        dev_addr = PERIPH_ADDR_21;
    }

    int ret = i2c_write_read(bus,
                             dev_addr,
                             &reg_addr, sizeof(reg_addr), /* write phase */
                             byte, sizeof(byte)); /* read phase  */
    if (ret < 0)
    {
        LOG_ERR("SLIC read failed (addr=0x%02x reg=0x%02x): %d (%s)",
                dev_addr, reg_addr, ret, strerror(-ret));
    }


    uint8_t buf[2];
    uint8_t results =0x0;

    buf[0] = reg_addr;
    results = (byte[0] & ~mode_mask) | (mode & mode_mask);
    buf[1] = results;

    //LOG_INF("byte: 0x%x, mask: 0x%x, mode: 0x%x, results: 0x%x", byte[0], mask, mode, results);

    ret = i2c_write(bus, buf, sizeof(buf), dev_addr);
    return ret;
}

/* ----------------------------------------------------------------*/
/* Function Sets Any Register in SLIC  */
/* -------------------------------------------------------------*/

int set_slic(const struct device* bus,
             uint8_t dev_addr, //Is the subscriber number
             uint8_t reg_addr,
             uint8_t mode,
             uint8_t mask)
{

    uint8_t byte[2] = {0x0};
    int ret = i2c_write_read(bus,
                             dev_addr,
                             &reg_addr, sizeof(reg_addr), /* write phase */
                             byte, sizeof(byte)); /* read phase  */
    if (ret < 0)
    {
        LOG_ERR("SLIC read failed (addr=0x%02x reg=0x%02x): %d (%s)",
                dev_addr, reg_addr, ret, strerror(-ret));
    }

    uint8_t buf[2];
    uint8_t results =0x0;

    buf[0] = reg_addr;
    results = (byte[0] & ~mask) | (mode & mask);
    buf[1] = results;

    //LOG_INF("byte: 0x%x, mask: 0x%x, mode: 0x%x, results: 0x%x", byte[0], mask, mode, results);

    ret = i2c_write(bus, buf, sizeof(buf), dev_addr);
    return ret;
}

/* ----------------------------------------------------------------*/
/* Function Sets Mode Register in SLIC  */
/* -------------------------------------------------------------*/

int set_slic_txmux(const struct device* bus,
             uint8_t device, //Is the subscriber number
             uint8_t val)
{
    uint8_t reg_addr = MCPREG_GPIO_A; //GPIO A
    uint8_t mode_mask = SLIC_MUX; //Bits 4-6
    uint8_t byte[2] = {0x0};
    uint8_t dev_addr;

    switch (device)
    {
    case 0x0:
        dev_addr = PERIPH_ADDR_20;
        break;
    case 0x1:
        dev_addr = PERIPH_ADDR_21;
        break;
    default:
        dev_addr = PERIPH_ADDR_21;
    }

    int ret = i2c_write_read(bus,
                             dev_addr,
                             &reg_addr, sizeof(reg_addr), /* write phase */
                             byte, sizeof(byte)); /* read phase  */
    if (ret < 0)
    {
        LOG_ERR("SLIC read failed (addr=0x%02x reg=0x%02x): %d (%s)",
                dev_addr, reg_addr, ret, strerror(-ret));
    }


    uint8_t buf[2];
    uint8_t results =0x0;

    buf[0] = reg_addr;
    results = (byte[0] & ~mode_mask) | (val & mode_mask);
    buf[1] = results;

    //LOG_INF("byte: 0x%x, mask: 0x%x, val: 0x%x, results: 0x%x", byte[0], mask, mode, results);

    ret = i2c_write(bus, buf, sizeof(buf), dev_addr);
    return ret;
}

int set_slic_rxmux(const struct device* bus,
             uint8_t device, //Is the subscriber number
             uint8_t val)
{
    uint8_t reg_addr = MCPREG_GPIO_B; //GPIO A
    uint8_t mode_mask = SLIC_MUX; //Bits 4-6
    uint8_t byte[2] = {0x0};
    uint8_t dev_addr;

    switch (device)
    {
    case 0x0:
        dev_addr = PERIPH_ADDR_20;
        break;
    case 0x1:
        dev_addr = PERIPH_ADDR_21;
        break;
    default:
        dev_addr = PERIPH_ADDR_21;
    }

    int ret = i2c_write_read(bus,
                             dev_addr,
                             &reg_addr, sizeof(reg_addr), /* write phase */
                             byte, sizeof(byte)); /* read phase  */
    if (ret < 0)
    {
        LOG_ERR("SLIC read failed (addr=0x%02x reg=0x%02x): %d (%s)",
                dev_addr, reg_addr, ret, strerror(-ret));
    }


    uint8_t buf[2];
    uint8_t results =0x0;

    buf[0] = reg_addr;
    results = (byte[0] & ~mode_mask) | (val & mode_mask);
    buf[1] = results;

    //LOG_INF("byte: 0x%x, mask: 0x%x, val: 0x%x, results: 0x%x", byte[0], mask, mode, results);

    ret = i2c_write(bus, buf, sizeof(buf), dev_addr);
    return ret;
}