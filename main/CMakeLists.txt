idf_component_register(
        SRCS "app_main.c"
        INCLUDE_DIRS "."
        REQUIRES driver dht display_lvgl
)
