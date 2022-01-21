## --------------------------------------------------------------------------
## Read BME280 sensor values with a Raspberry Pi Pico using the official Bosch-API
##
## Author: Bernhard Bablok
##
## https://github.com/bablokb/pico-bme280
## --------------------------------------------------------------------------

# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
#set(PICO_SDK_PATH "/path/to/sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico-bme280 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(pico-bme280 pico-bme280.c bme280.c user.c)

add_compile_definitions(SPI_PORT=spi1)
add_compile_definitions(SPI_RX=12)
add_compile_definitions(SPI_CS=13)
add_compile_definitions(SPI_SCK=14)
add_compile_definitions(SPI_TX=15)
add_compile_definitions(ALTITUDE_AT_LOC=520)
add_compile_definitions(BME280_32BIT_ENABLE)

# configuration for the st7735-library
add_compile_definitions(SPI_TFT_CS=9)
add_compile_definitions(SPI_TFT_DC=10)
add_compile_definitions(SPI_TFT_RST=11)
add_compile_definitions(TFT_ENABLE_RESET TFT_ENABLE_BLACK TFT_ENABLE_TEXT
                        TFT_ENABLE_SHAPES TFT_ENABLE_FONTS)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_VERBOSE_MAKEFILE 1)
  add_compile_definitions(DEBUG)
  add_compile_definitions(UPDATE_INTERVAL=10)
  pico_enable_stdio_uart(pico-bme280 1)
  pico_enable_stdio_usb(pico-bme280 0)
else()
  if(LOG STREQUAL "USB")
    add_compile_definitions(LOG)
    pico_enable_stdio_uart(pico-bme280 0)
    pico_enable_stdio_usb(pico-bme280 1)
    add_compile_definitions(UPDATE_INTERVAL=10)
  elseif(LOG STREQUAL "SERIAL")
    add_compile_definitions(LOG)
    pico_enable_stdio_uart(pico-bme280 1)
    pico_enable_stdio_usb(pico-bme280 0)
    add_compile_definitions(UPDATE_INTERVAL=10)
  else()
    add_compile_definitions(UPDATE_INTERVAL=60)
  endif()
endif()

pico_set_program_name(pico-bme280 "pico-bme280")
pico_set_program_version(pico-bme280 "0.1")

# build the st7735-library from our submodule
add_subdirectory(pico-st7735/lib-st7735)

# Add the standard library to the build
target_link_libraries(pico-bme280 pico_stdlib
                                  lib-st7735
                                  hardware_spi)

pico_add_extra_outputs(pico-bme280)

