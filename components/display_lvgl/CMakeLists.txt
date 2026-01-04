idf_component_register(SRCS "display_lvgl.c"
        INCLUDE_DIRS "include"
        REQUIRES
        esp_lvgl_port
        lvgl
        freertos
        esp_driver_gpio
        esp_driver_spi)
