//
// Created by Hans Gelke on 21.06.2026.
//

#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include "tones.h"
#include "spio.h"
#include "main.h"
#include "gpio_i2c.h"

LOG_MODULE_REGISTER(tone_generator, LOG_LEVEL_DBG);

/* tones.c-specific mask constants not defined elsewhere              */
#define MASK_LOW        0xF0
#define MASK_NO         0xFF
#define I2C_OUTPUT      0x00    /* MCP23017: all pins as outputs      */
#define I2C_INPUT       0xFF    /* MCP23017: all pins as inputs       */

/* ------------------------------------------------------------------ */
/* Helper — write a CMX865 register and log the readback              */
/* ------------------------------------------------------------------ */
static void cmx865_write_verify(const uint8_t *cmd, size_t len,
                                uint8_t readback_reg)
{
    uint16_t val = 0;
    spi_write_bytes(cmd, len);
    k_msleep(10);
    spi_read_register(readback_reg, &val);
    LOG_INF("Reg 0x%02X readback: 0x%02X", readback_reg, val);
}

/* ------------------------------------------------------------------ */
/* CMX865 initialisation                                               */
/* ------------------------------------------------------------------ */
int initCMX865(void)
{
    const struct spi_dt_spec spi_dev =
        SPI_DT_SPEC_GET(SPI_DEV_NODE,
                        SPI_OP_MODE_MASTER |
                        SPI_TRANSFER_MSB   |
                        SPI_WORD_SET(8)    |
                        SPI_LINES_SINGLE);

    //const struct device* i2c_bus0 = DEVICE_DT_GET(I2C_NODE_0);

    LOG_INF("CMX865 init starting");

    // ----------------------------------------------------------------
    // Set controll register to output to I2C Device address 0x24
    //----------------------------------------------------------------
    set_slic(i2c_bus0,
             PERIPH_ADDR_24,
             IODIR_A,
             I2C_OUTPUT,
             MASK_NO);

    set_slic(i2c_bus0,
             PERIPH_ADDR_24,
             IODIR_B,
             I2C_OUTPUT,
             MASK_NO);

    set_slic(i2c_bus0,
             PERIPH_ADDR_24,
             GPIO_B,
             0x01,//Set Address decoder to 1 for CMX no 1
             MASK_HIGH);

    if (!spi_is_ready_dt(&spi_dev)) {
        LOG_ERR("SPI device not ready");
        return -ENODEV;
    }

    //--------------------------------
    // Read registers 0x24
    //----------------------------------------------------------------

    uint8_t rx_buf20[2] = {0};
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


    const uint8_t READBACK_REG = 0xE6;




    /* General reset                                                   */
    static const uint8_t general_reset[]  = { 0x01 };
    spi_write_bytes(general_reset, sizeof(general_reset));
    k_msleep(10);

    /* Set reset bit                                                    */
    static const uint8_t set_reset[]      = { 0xE0, 0x0D, 0x80 };
    spi_write_bytes(set_reset, sizeof(set_reset));
    k_msleep(20);

    /* Clear reset bit                                                  */
    static const uint8_t clear_reset[]    = { 0xE0, 0x0D, 0x00 };
    spi_write_bytes(clear_reset, sizeof(clear_reset));
    k_msleep(10);

    /* TX mode register                                                 */
    static const uint8_t tx_mode[]        = { 0xE1, 0x1E, 0x00 };
    spi_write_bytes(tx_mode, sizeof(tx_mode));
    k_msleep(10);

    /* Tone generator program start                                     */
    static const uint8_t prog_start[]     = { 0xE8, 0x80, 0x00 };
    spi_write_bytes(prog_start, sizeof(prog_start));
    k_msleep(10);

    /* Frequency 1                                                      */
    static const uint8_t freq_1[]         = { 0xE8, 0x05, 0xDE };
    cmx865_write_verify(freq_1, sizeof(freq_1), READBACK_REG);

    /* Level 1                                                          */
    static const uint8_t level_1[]        = { 0xE8, 0x37, 0x81 };
    cmx865_write_verify(level_1, sizeof(level_1), READBACK_REG);

    /* Frequency 2                                                      */
    static const uint8_t freq_2[]         = { 0xE8, 0x05, 0xDE };
    cmx865_write_verify(freq_2, sizeof(freq_2), READBACK_REG);

    /* Level 2                                                          */
    static const uint8_t level_2[]        = { 0xE8, 0x37, 0x81 };
    cmx865_write_verify(level_2, sizeof(level_2), READBACK_REG);

    /* Enable TX output                                                 */
    static const uint8_t tx_enable[]      = { 0xE1, 0x1E, 0x0C };
    spi_write_bytes(tx_enable, sizeof(tx_enable));
    k_msleep(10);

    LOG_INF("CMX865 init done");
    return 0;
}
