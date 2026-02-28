#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_tls.h"

#include "mqtt_client.h"

static const char *TAG = "mosquitto";
#include "sdkconfig.h"
#include "mosquitto.h"

esp_mqtt_client_handle_t s_client = NULL;

static void log_error_if_nonzero(const char *msg, int error_code) {
    if (error_code != 0 ) {
        ESP_LOGE(TAG, "%s: 0x%x", msg, error_code);
    }
}
static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {

    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;


    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "sensors/temp", 0);
            ESP_LOGI(TAG, "sent subscribe, msg_id=%d", msg_id);
            msg_id = esp_mqtt_client_publish(client, "status", "online", 0, 1, 0);
            ESP_LOGI(TAG, "sent publish, msg_id=%d", msg_id);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGI(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "DATA=%.*s", event->data_len, event->data);
            break;

        case MQTT_EVENT_ERROR: {
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
            esp_mqtt_error_codes_t *err = event->error_handle;
            if (err) {
                log_error_if_nonzero("reported from esp-tls", err->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", err->esp_tls_stack_err);
                log_error_if_nonzero("socket errno", err->esp_transport_sock_errno);
            }
            break;
        }

        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
        .credentials.username = CONFIG_MQTT_USERNAME,
        .credentials.authentication.password = CONFIG_MQTT_PASSWORD,
        .session.keepalive = 120,
        .session.disable_keepalive = false
    };

    s_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID,
                                   mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_client);
}
