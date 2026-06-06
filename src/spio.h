//
// Created by Hans Gelke on 05.06.2026.
//

#ifndef SRC_SPIO_H
#define SRC_SPIO_H

#include <stdint.h>
#include <stddef.h>

int spi_write_bytes(const uint8_t *data, size_t len);

int spi_read_bytes(uint8_t *data, size_t len);

int spi_read_register(uint8_t reg_addr, uint16_t *value);

int spi_transceive_bytes(const uint8_t *tx_data,
                                uint8_t       *rx_data,
                                size_t         len);

#endif //SRC_SPIO_H