#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "esp_crc.h"
#include "esp_mac.h"
#include "espnow_handler.h"
#include "sensors_types.h"
static const char *TAG = "ESP-NOW";

static uint8_t s_seq = 0;
static const uint8_t MASTER_MAC[6] = {0x00, 0x4b, 0x12, 0x3a, 0x9b, 0x58};

static uint16_t calc_crc(const void *data, size_t len) {
    return esp_crc16_le(UINT16_MAX, (const uint8_t *)data, len);
}

void on_data_sent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {

    char master_str[18];
    snprintf(master_str, sizeof(master_str), MACSTR, MAC2STR(tx_info->des_addr));
        
ESP_LOGI(TAG, "Send to %s — status: %d", master_str, (int)status);
    if (status == ESP_NOW_SEND_SUCCESS) {
        ESP_LOGI(TAG, "Data sent successfully");
    } else {
        ESP_LOGE(TAG, "Data send failed");
    }
}

void espnow_handler_init(void) {
    char master_str[18];

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(on_data_sent));

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, MASTER_MAC, 6);
    peer.channel = 0;
    peer.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

    snprintf(master_str, sizeof(master_str), MACSTR, MAC2STR(MASTER_MAC));
    ESP_LOGI(TAG, "ESP-NOW slave initialized, master: %s", master_str);

}

void espnow_send_telemetry(const SensorData_t *data) {
    espnow_packet_t pkt = {
        .type = PKT_TELEMETRY,
        .seq  = s_seq++,
        .crc  = 0,
    };
    memcpy(&pkt.payload, data, sizeof(SensorData_t));
    pkt.crc = calc_crc(&pkt.payload, sizeof(SensorData_t));

    esp_err_t ret = esp_now_send(MASTER_MAC, (uint8_t *)&pkt, sizeof(pkt));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_now_send failed: %s", esp_err_to_name(ret));
    }
}