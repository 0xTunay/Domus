idf_component_register(SRCS "mosquitto.c"
        INCLUDE_DIRS "include"
        REQUIRES mqtt driver nvs_flash esp_netif protocol_examples_common)