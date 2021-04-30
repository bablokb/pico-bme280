#include "user.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"

void user_delay_us(uint32_t period, void *intf_ptr) {
  sleep_ms(period);
}

int8_t user_spi_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) {
  int8_t rslt = 0;
  gpio_put(SPI_CS, 0);                        // Chip select is active-low
  spi_write_blocking(SPI_PORT,&reg_addr,1);
  spi_read_blocking(SPI_PORT,0,reg_data,len);
  gpio_put(SPI_CS, 1);                        // Chip select is active-low
  return rslt;
}

int8_t user_spi_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) {
  int8_t rslt = 0;
  gpio_put(SPI_CS, 0);                        // Chip select is active-low
  spi_write_blocking(SPI_PORT,&reg_addr,1);
  spi_write_blocking(SPI_PORT,reg_data,len);
  gpio_put(SPI_CS, 1);                        // Chip select is active-low
  return rslt;
}
