//
// Created by Hans Gelke on 05.06.2026.
//

#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include "spio.h"

#define SPI_DEV_NODE DT_NODELABEL(spi_dev0)

LOG_MODULE_REGISTER(spi_example, LOG_LEVEL_DBG);


const struct spi_dt_spec spi_dev =
    SPI_DT_SPEC_GET(SPI_DEV_NODE,
                    SPI_OP_MODE_MASTER |
                    SPI_TRANSFER_MSB   |
                    SPI_WORD_SET(8)    |
                    SPI_LINES_SINGLE);

int spi_write_bytes(const uint8_t *data, size_t len)
{
    struct spi_buf     tx_buf = { .buf = (void *)data, .len = len };
    struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1  };

    int ret = spi_write_dt(&spi_dev, &tx_set);
    if (ret < 0) { LOG_ERR("spi_write_dt failed: %d", ret); }
    return ret;
}

int spi_read_bytes(uint8_t *data, size_t len)
{
    struct spi_buf     rx_buf = { .buf = data, .len = len };
    struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

    int ret = spi_read_dt(&spi_dev, &rx_set);
    if (ret < 0) { LOG_ERR("spi_read_dt failed: %d", ret); }
    return ret;
}

int spi_transceive_bytes(const uint8_t *tx_data,
                                uint8_t       *rx_data,
                                size_t         len)
{
    struct spi_buf     tx_buf = { .buf = (void *)tx_data, .len = len };
    struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1  };
    struct spi_buf     rx_buf = { .buf = rx_data,        .len = len };
    struct spi_buf_set rx_set = { .buffers = &rx_buf,    .count = 1 };

    int ret = spi_transceive_dt(&spi_dev, &tx_set, &rx_set);
    if (ret < 0) { LOG_ERR("spi_transceive_dt failed: %d", ret); }
    return ret;
}

int spi_read_register(uint8_t reg_addr, uint16_t *value)
{
    uint8_t tx[3] = { reg_addr, 0x00u, 0x00u };
    uint8_t rx[3] = { 0, 0, 0 };

    int ret = spi_transceive_bytes(tx, rx, sizeof(tx));
    if (ret == 0) { *value = ((uint16_t)rx[1] << 8) | rx[2]; }
    return ret;
}