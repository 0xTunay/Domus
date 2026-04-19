idf_component_register(SRCS "ota_receiver.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_wifi app_update esp_common freertos)
