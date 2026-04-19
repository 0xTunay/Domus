idf_component_register(SRCS "ota_sender.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_wifi app_update)
