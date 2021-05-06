// --------------------------------------------------------------------------
// Declaration of user-supplied functions for Bosch-API
//
// Author: Bernhard Bablok
//
// https://github.com/bablokb/pico-bme280
// --------------------------------------------------------------------------

#ifndef _USER_H
#define _USER_H

#include "pico/stdlib.h"

#ifndef SPI_PORT
  #define SPI_PORT spi1
#endif
#ifndef SPI_RX
  #define SPI_RX  12
#endif
#ifndef SPI_CS
  #define SPI_CS  13
#endif
#ifndef SPI_SCK
  #define SPI_SCK 14
#endif
#ifndef SPI_TX
  #define SPI_TX  15
#endif

void user_delay_us(uint32_t period, void *intf_ptr);
int8_t user_spi_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t user_spi_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);

#endif
