#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "esp_crc.h"
#include "esp_mac.h"
#include "espnow_handler.h"
#include "Sensors_types.h"

static const char *TAG = "ESP-NOW";

extern QueueHandle_t SensorQueueHandle;

static uint16_t calc_crc(const void *data, size_t len) {
    return esp_crc16_le(UINT16_MAX, (const uint8_t *)data, len);
}

static void on_data_recv(const esp_now_recv_info_t *info,
                         const uint8_t *data, int len)
{
    if (len != sizeof(espnow_packet_t)) {
        ESP_LOGW(TAG, "Wrong packet size: got %d, expected %d",
                 len, sizeof(espnow_packet_t));
        return;
    }

    espnow_packet_t *pkt = (espnow_packet_t *)data;

    if (pkt->type != PKT_TELEMETRY) {
        ESP_LOGW(TAG, "Unknown packet type: %d", pkt->type);
        return;
    }

    uint16_t crc = calc_crc(&pkt->payload, sizeof(SensorData_t));
    if (crc != pkt->crc) {
        ESP_LOGE(TAG, "CRC mismatch! got=0x%04X expected=0x%04X",
                 pkt->crc, crc);
        return;
    }
    SensorData_t sensor_data;
    memcpy(&sensor_data, &pkt->payload, sizeof(SensorData_t));

    ESP_LOGI(TAG, "Received seq=%d temp=%"PRIu32" hum=%"PRIu32" press=%"PRIu32,
             pkt->seq,
             sensor_data.temperature,
             sensor_data.humidity,
             sensor_data.pressure);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (xQueueSendFromISR(SensorQueueHandle, &sensor_data,
                          &xHigherPriorityTaskWoken) != pdTRUE) {
        ESP_LOGW(TAG, "Queue full, packet dropped!");
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void espnow_master_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(esp_now_init());

    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_data_recv));

    ESP_LOGI(TAG, "ESP-NOW master initialized");
}