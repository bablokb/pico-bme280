#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "user.h"
#include "bme280.h"

void init_hw() {
  stdio_init_all();
  spi_init(SPI_PORT, 1000000);                // SPI with 1Mhz
  gpio_set_function(SPI_RX, GPIO_FUNC_SPI);
  gpio_set_function(SPI_SCK,GPIO_FUNC_SPI);
  gpio_set_function(SPI_TX, GPIO_FUNC_SPI);

  gpio_init(SPI_CS);
  gpio_set_dir(SPI_CS, GPIO_OUT);
  gpio_put(SPI_CS, 1);                        // Chip select is active-low
}

int8_t init_sensor(struct bme280_dev *dev, uint32_t *delay) {
  int8_t rslt = BME280_OK;
  uint8_t settings;
  // basic initialization
  dev->intf_ptr = SPI_PORT;       // SPI_PORT is an address
  dev->intf     = BME280_SPI_INTF;
  dev->read     = user_spi_read;
  dev->write    = user_spi_write;
  dev->delay_us = user_delay_us;
  rslt = bme280_init(dev);
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

int8_t read_sensor(struct bme280_dev *dev, uint32_t *delay, struct bme280_data *data) {
  int8_t rslt;
  rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, dev);
  if (rslt != BME280_OK) {
    return rslt;
  }
  dev->delay_us(*delay,dev->intf_ptr);
  return bme280_get_sensor_data(BME280_ALL,data,dev);
}

void print_data(struct bme280_data *data) {
  float temp, press, hum;
  temp  = 0.01f * data->temperature;
  press = 0.01f * data->pressure;
  hum   = 1.0f / 1024.0f * data->humidity;
  printf("%0.2lf deg C, %0.2lf hPa, %0.2lf%%\n", temp, press, hum);
}

int main() {
  struct bme280_dev dev;
  struct bme280_data sensor_data;
  int8_t rslt;
  uint32_t delay;               // calculated delay between measurements
  init_hw();
  rslt = init_sensor(&dev,&delay);
  if (rslt != BME280_OK) {
    printf("could not initialize sensor. RC: %d\n", rslt);
  } else {
    printf("Temperature, Pressure, Humidity\n");
    while (read_sensor(&dev,&delay,&sensor_data) == BME280_OK) {
      print_data(&sensor_data);
      sleep_ms(1000);
    }
    printf("error while reading sensor: RC: %d", rslt);
  }
  return 0;
}