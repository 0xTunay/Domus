#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "mqtt_client.h"

static const char *TAG = "mosquitto";

#include "mosquitto.h"
// void mqtt_event_handler(void *handler_args,
//                                esp_event_base_t base,
//                                int32_t event_id,
//                                void *event_data);

void mqtt_app_start(void) {
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    // esp_mqtt_client_register_event(client,
    //                                ESP_EVENT_ANY_ID,
    //                                esp_mqtt_event_handle_t,
    //                                NULL);
    esp_mqtt_client_start(client);
}
