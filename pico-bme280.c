// --------------------------------------------------------------------------
// Read BME280 sensor values with a Raspberry Pi Pico using the official Bosch-API
//
// Author: Bernhard Bablok
//
// https://github.com/bablokb/pico-bme280
// --------------------------------------------------------------------------

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "user.h"
#include "bme280.h"
#include "ST7735_TFT.h"

// ---------------------------------------------------------------------------
// hardware-specific intialization
// SPI_* constants from CMakeLists.txt or user.h

void init_hw() {
  stdio_init_all();
  spi_init(SPI_PORT, 1000000);                // SPI with 1Mhz
  gpio_set_function(SPI_RX, GPIO_FUNC_SPI);
  gpio_set_function(SPI_SCK,GPIO_FUNC_SPI);
  gpio_set_function(SPI_TX, GPIO_FUNC_SPI);

  gpio_init(SPI_CS);
  gpio_set_dir(SPI_CS, GPIO_OUT);
  gpio_put(SPI_CS, 1);                        // Chip select is active-low

  gpio_init(SPI_TFT_CS);
  gpio_set_dir(SPI_TFT_CS, GPIO_OUT);
  gpio_put(SPI_TFT_CS, 1);                        // Chip select is active-low

  gpio_init(SPI_TFT_DC);
  gpio_set_dir(SPI_TFT_DC, GPIO_OUT);
  gpio_put(SPI_TFT_DC, 0);                        // Chip select is active-low

  gpio_init(SPI_TFT_RST);
  gpio_set_dir(SPI_TFT_RST, GPIO_OUT);
  gpio_put(SPI_TFT_RST, 0);
}

// ---------------------------------------------------------------------------
// initialize TFT
// Background plus three fields

void init_tft() {
  #ifdef DEBUG
    printf("[DEBUG] initializing TFT\n");
  #endif
  TFT_BlackTab_Initialize();
  fillScreen(ST7735_BLUE);
  fillRoundRect(4,  4,120,48,10,ST7735_WHITE);
  fillRoundRect(4, 56,120,48,10,ST7735_WHITE); //  4 + 48 + 4
  fillRoundRect(4,108,120,48,10,ST7735_WHITE); // 56 + 48 + 4
}

// ---------------------------------------------------------------------------
// initialize sensor

int8_t init_sensor(struct bme280_dev *dev, uint32_t *delay) {
  int8_t rslt = BME280_OK;
  uint8_t settings;

  // give sensor time to startup
  sleep_ms(5);                    // datasheet: 2ms

  // basic initialization
  dev->intf_ptr = SPI_PORT;       // SPI_PORT is an address
  dev->intf     = BME280_SPI_INTF;
  dev->read     = user_spi_read;
  dev->write    = user_spi_write;
  dev->delay_us = user_delay_us;
  rslt = bme280_init(dev);
#ifdef DEBUG
  printf("[DEBUG] chip-id: 0x%x\n",dev->chip_id);
#endif
  if (rslt != BME280_OK) {
    return rslt;
  }
  // configure for forced-mode, indoor navigation
  dev->settings.osr_h = BME280_OVERSAMPLING_1X;
  dev->settings.osr_p = BME280_OVERSAMPLING_16X;
  dev->settings.osr_t = BME280_OVERSAMPLING_2X;
  dev->settings.filter = BME280_FILTER_COEFF_16;

  settings = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;
  if (rslt != BME280_OK) {
    return rslt;
  }
  rslt   = bme280_set_sensor_settings(settings,dev);
  *delay = bme280_cal_meas_delay(&dev->settings);
  return rslt;
}

// ---------------------------------------------------------------------------
// read sensor values

int8_t read_sensor(struct bme280_dev *dev, uint32_t *delay,
                   struct bme280_data *data) {
  int8_t rslt;
  rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, dev);
  if (rslt != BME280_OK) {
    return rslt;
  }
  dev->delay_us(*delay,dev->intf_ptr);
  return bme280_get_sensor_data(BME280_ALL,data,dev);
}

// ---------------------------------------------------------------------------
// print sensor data to console

void print_data(struct bme280_data *data) {
  float temp, press, hum;
  float alt_fac = pow(1.0-ALTITUDE_AT_LOC/44330.0, 5.255);

  temp  = 0.01f * data->temperature;
  press = 0.01f * data->pressure/alt_fac;
  hum   = 1.0f / 1024.0f * data->humidity;
  printf("%0.1f deg C, %0.0f hPa, %0.0f%%\n", temp, press, hum);
}

// ---------------------------------------------------------------------------
// main loop: read data and print data to console

int main() {
  struct bme280_dev dev;
  struct bme280_data sensor_data;
  int8_t rslt;
  uint32_t delay;               // calculated delay between measurements
  init_hw();
  init_tft();
  rslt = init_sensor(&dev,&delay);
  if (rslt != BME280_OK) {
    printf("could not initialize sensor. RC: %d\n", rslt);
  } else {
    printf("Temperature, Pressure, Humidity\n");
    while (read_sensor(&dev,&delay,&sensor_data) == BME280_OK) {
      print_data(&sensor_data);
      sleep_ms(1000*UPDATE_INTERVAL);
    }
    printf("error while reading sensor: RC: %d", rslt);
  }
  return 0;
}
