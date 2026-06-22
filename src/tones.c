//
// Created by Hans Gelke on 21.06.2026.
//
#include <zephyr/drivers/spi.h>
    #include <zephyr/logging/log.h>

#include "tones.h"
#include "spio.h"
#include "main.h"


LOG_MODULE_REGISTER(tone_generator, LOG_LEVEL_DBG);
#define SPI_DEV_NODE DT_NODELABEL(spi_dev0)

int initCMX865(void)
{
    //----------------------------------------------------------------
    // Initialize SPI
    //----------------------------------------------------------------
    const struct spi_dt_spec spi_dev =
        SPI_DT_SPEC_GET(SPI_DEV_NODE,
                        SPI_OP_MODE_MASTER |
                        SPI_TRANSFER_MSB   |
                        SPI_WORD_SET(8)    |
                        SPI_LINES_SINGLE);


    // ----------------------------------------------------------------
    // Prepare CMX865 via SPIO
    //----------------------------------------------------------------
    LOG_INF("SPI0 example starting on RP2350");

    if (!spi_is_ready_dt(&spi_dev)) {
        LOG_ERR("SPI device not ready");
        return -ENODEV;
    }
    //General Reset
    uint8_t cmd3[] = { 0x01 };
    spi_write_bytes(cmd3, sizeof(cmd3));
    k_msleep(10);
    //Set Reset bit
    uint8_t cmd1[] = { 0xE0, 0x0D, 0x80 };

    spi_write_bytes(cmd1, sizeof(cmd1));
    k_msleep(20);
    //Clear Reset bit
    uint8_t cmd2[] = { 0xE0, 0x0D, 0x00 };
    //while (1) {
    spi_write_bytes(cmd2, sizeof(cmd2));
    //}
    k_msleep(10);
    //tx mode register
    uint8_t cmd4[] = { 0xE1, 0x1E, 0x00 };
    //while (1) {
    spi_write_bytes(cmd4, sizeof(cmd4));
    //}
    uint16_t val = 0;
    const uint8_t REG = 0xE6;
    k_msleep(10);
    uint8_t prog0[] = { 0xE8, 0x80, 0x00 };
    spi_write_bytes(prog0, sizeof(prog0));
    k_msleep(10);
    uint8_t freq_1[] = { 0xE8, 0x05, 0xde };
    spi_write_bytes(freq_1, sizeof(freq_1));
    k_msleep(10);
    //while (1) {
    spi_read_register(REG, &val);
    //}
    LOG_INF("Reg 0x%02X readback: 0x%02X", REG, val);
    k_msleep(10);
    uint8_t levl_1[] = { 0xE8, 0x37, 0x81 };
    spi_write_bytes(levl_1, sizeof(levl_1));
    k_msleep(10);
    spi_read_register(REG, &val);
    LOG_INF("Reg 0x%02X readback: 0x%02X", REG, val);
    k_msleep(10);
    uint8_t freq_2[] = { 0xE8, 0x05, 0xde };
    spi_write_bytes(freq_2, sizeof(freq_2));
    k_msleep(10);
    spi_read_register(REG, &val);
    LOG_INF("Reg 0x%02X readback: 0x%02X", REG, val);
    k_msleep(10);
    uint8_t levl_2[] = { 0xE8, 0x37, 0x81 };
    spi_write_bytes(levl_2, sizeof(levl_2));
    k_msleep(10);
    spi_read_register(REG, &val);
    LOG_INF("Reg 0x%02X readback: 0x%02X", REG, val);
    k_msleep(10);

    spi_write_bytes(freq_1, sizeof(freq_1));
    k_msleep(10);
    spi_read_register(REG, &val);
    k_msleep(10);
    LOG_INF("Reg 0x%02X readback: 0x%02X", REG, val);
    k_msleep(10);
    spi_write_bytes(levl_1, sizeof(levl_1));
    k_msleep(10);
    spi_write_bytes(freq_2, sizeof(freq_2));
    k_msleep(10);
    spi_write_bytes(levl_2, sizeof(levl_2));
    k_msleep(10);

    spi_write_bytes(freq_1, sizeof(freq_1));
    k_msleep(10);
    spi_write_bytes(levl_1, sizeof(levl_1));
    k_msleep(10);
    spi_write_bytes(freq_2, sizeof(freq_2));
    k_msleep(10);
    spi_write_bytes(levl_2, sizeof(levl_2));
    k_msleep(10);

    spi_write_bytes(freq_1, sizeof(freq_1));
    k_msleep(10);
    spi_write_bytes(levl_1, sizeof(levl_1));
    k_msleep(10);
    spi_write_bytes(freq_2, sizeof(freq_2));
    k_msleep(10);
    spi_write_bytes(levl_2, sizeof(levl_2));
    k_msleep(10);
    spi_read_register(REG, &val);
    LOG_INF("Reg 0x%02X readback: 0x%02X", REG, val);

    uint8_t cmd5[] = { 0xE1, 0x1E, 0x0c };
    spi_write_bytes(cmd5, sizeof(cmd5));
    LOG_INF("Done");

    return 0; // Return from main.c
}