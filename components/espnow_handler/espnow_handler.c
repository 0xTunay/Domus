#include <string.h>
#include "esp_log.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "espnow_handler.h"
#include "receive_queue.h"
#include "Sensors_types.h"

static const char *TAG = "[espnow_handler]";

QueueHandle_t SensorQueueHandle = NULL;
static const uint8_t SLAVE_MAC[6] = {0xe8, 0x3d, 0xc1, 0x8c, 0x04, 0xfc};

static void espnow_recv_cb(const esp_now_recv_info_t *recv_info,
                           const uint8_t *data,
                           int data_len)
{
    if (data == NULL || data_len != sizeof(espnow_packet_t)) {
        ESP_LOGW(TAG, "Invalid packet: len=%d expected=%d",
                 data_len, (int)sizeof(espnow_packet_t));
        return;
    }

    const espnow_packet_t *pkt = (const espnow_packet_t *)data;


    switch (pkt->type) {

    case PKT_TELEMETRY: {
        const SensorData_t *s = &pkt->payload;

        ESP_LOGI(TAG, "Telemetry from " MACSTR ": T=%.1f H=%.1f P=%.1f",
                 MAC2STR(recv_info->src_addr),
                 s->temperature, s->humidity, s->pressure);

        if (xQueueSendToBack(TemperatureQueueHandle,
                             &s->temperature, 0) != pdTRUE) {
            ESP_LOGW(TAG, "TemperatureQueue full, drop");
        }

        if (xQueueSendToBack(HumidityQueueHandle,
                             &s->humidity, 0) != pdTRUE) {
            ESP_LOGW(TAG, "HumidityQueue full, drop");
        }

        if (xQueueSendToBack(PressureQueueHandle,
                             &s->pressure, 0) != pdTRUE) {
            ESP_LOGW(TAG, "PressureQueue full, drop");
        }
        break;
    }

    case PKT_OTA_BEGIN:
    case PKT_OTA_DATA:
    case PKT_OTA_END:
        /* TODO: OTA handler */
        ESP_LOGI(TAG, "OTA packet type=0x%02x, seq=%d", pkt->type, pkt->seq);
        break;

    default:
        ESP_LOGW(TAG, "Unknown packet type: 0x%02x", pkt->type);
        break;
    }
}

void espnow_master_init(void)
{
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));

    esp_now_peer_info_t peer = {0};
    memcpy(peer.peer_addr, SLAVE_MAC, ESP_NOW_ETH_ALEN);
    peer.channel = 6;
    peer.ifidx   = WIFI_IF_STA;
    peer.encrypt = false;

    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

    ESP_LOGI(TAG, "ESP-NOW master ready");
}