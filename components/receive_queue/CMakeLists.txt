idf_component_register(SRCS "receive_queue.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_wifi nvs_flash bt
                    REQUIRES driver mosquitto wifi_manager espnow_handler receive_queue)
