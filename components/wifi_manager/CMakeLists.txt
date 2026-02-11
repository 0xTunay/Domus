idf_component_register(SRCS "wifi_manager.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_wifi nvs_flash bt
                    REQUIRES esp_wifi esp_event esp_netif)
