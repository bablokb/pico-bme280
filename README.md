pico-bme280
===========

This is an example program that reads BME280 sensor values with a Raspberry Pi Pico using the official Bosch-API (SPI-interface).

Note that about 99% of the sellers offer a "BME280/BMP280"-breakout, but instead of
a BME280 (chip-id 0x60), you will most probably receive a cheaper BMP280 (chip-id 0x56, 0x57 or 0x58). The former provides readouts for temperature, pressure and humidity, while the latter lacks humidity.

You will find a similar project for the BMP280 in
<https://github.com/bablokb/pico-bmp280>.


License
-------

My code is licensed using the GPL v3, but it uses the files from Bosch-Sensortec
<https://github.com/BoschSensortec/BME280_driver.git>, which is licensed under BSD-3.

