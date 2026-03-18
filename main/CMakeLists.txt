idf_component_register(
        SRCS "app_main.c"
        INCLUDE_DIRS "."
        PRIV_REQUIRES esp_wifi nvs_flash bt
        REQUIRES driver BME280 espnow_handler
)
