idf_component_register(SRCS "espnow_handler.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash esp_event esp_netif esp_wifi ota_receiver)
