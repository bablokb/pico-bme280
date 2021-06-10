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
#include "FreeMonoOblique12pt7b.h"

#define FIELDS     3
#define FIELD_W  110
#define FIELD_H   36
#define FIELD_R    5
#define TEXT_X     8
#define TEXT_Y    26
#define TEXT_FG   ST7735_BLACK
#define TEXT_BG   ST7735_WHITE
#define TFT_BG    ST7735_BLUE

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
  fillScreen(TFT_BG);
  setFont(&FreeMonoOblique12pt7b);
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
#ifdef DEBUG
  printf("[DEBUG] delay: %u µs\n",*delay);
#endif
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
// display sensor data on TFT

void display_data(struct bme280_data *data) {
  char values[3][10];
  float alt_fac = pow(1.0-ALTITUDE_AT_LOC/44330.0, 5.255);

  snprintf(values[0],10,"%+0.1f°C",0.01f * data->temperature);
  snprintf(values[1],10,"%0.0fhPa",0.01f * data->pressure/alt_fac);
  snprintf(values[2],10,"%0.0f%%",1.0f / 1024.0f * data->humidity);

  // clear output area
  uint8_t hgap = (tft_width-FIELD_W)/2;
  uint8_t vgap = (tft_height-FIELDS*FIELD_H)/(FIELDS+1);
  uint8_t y    = vgap;

  for (uint8_t i=0; i<FIELDS; ++i) {
    fillRoundRect(hgap,y,FIELD_W,FIELD_H,FIELD_R,TEXT_BG);
    y += FIELD_H+vgap;
  }

  // write sensor readouts
  y = vgap + TEXT_Y;
  for (uint8_t i=0; i<FIELDS; ++i) {
    drawText(hgap+TEXT_X,y,values[i],TEXT_FG,TEXT_BG,1);
    y += FIELD_H+vgap;
  }
}

// ---------------------------------------------------------------------------
// main loop: read data and display on TFT

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
      display_data(&sensor_data);
      sleep_ms(1000*UPDATE_INTERVAL);
    }
    printf("error while reading sensor: RC: %d", rslt);
  }
  return 0;
}
