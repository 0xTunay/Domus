idf_component_register(
    SRCS "BME280.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_i2c bme280 unity freertos log
)