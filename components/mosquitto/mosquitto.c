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

esp_mqtt_client_handle_t esp_mqtt_client_init(const esp_mqtt_client_config_t *config) {

    esp_mqtt_client_config_t mqttCfg;
    memset(&mqttCfg, 0, sizeof(esp_mqtt_client_config_t));
    return 0;
}