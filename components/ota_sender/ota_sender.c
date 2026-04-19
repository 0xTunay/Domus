#include "esp_now.h"
#include "esp_ota_ops.h"
#include "esp_log.h"
#include "esp_crc.h"
#include "string.h"
#include "esp_wifi.h"

#include "ota_sender.h"

static esp_ota_handle_t    s_ota_handle     = 0;
static const esp_partition_t *s_ota_part    = NULL;
static uint32_t            s_expected_seq   = 0;
static uint8_t             s_gw_mac[6]      = {0x00, 0x4b, 0x12, 0x3a, 0x9b, 0x58};
static bool                s_ota_in_progress = false;

static void send_response(uint8_t cmd, uint32_t seq) {
    ota_pkt_t pkt = {
        .cmd = cmd,
        .seq = seq,
    };
    esp_now_send(s_gw_mac, (uint8_t *)&pkt, sizeof(pkt));
}

void ota_rx_on_recv(const esp_now_recv_info_t *info,
                    const uint8_t *data, int len) {
    if (len < (int)offsetof(ota_pkt_t, data)) return;
    ota_pkt_t *pkt = (ota_pkt_t *)data;

    switch (pkt->cmd) {

    case OTA_CMD_START:
        memcpy(s_gw_mac, info->src_addr, 6);
        s_ota_part = esp_ota_get_next_update_partition(NULL);
        if (!s_ota_part) {
            ESP_LOGE(TAG, "No OTA partition found!");
            send_response(OTA_CMD_NACK, 0);
            return;
        }
        esp_ota_begin(s_ota_part, pkt->total_size, &s_ota_handle);
        s_expected_seq   = 0;
        s_ota_in_progress = true;
        ESP_LOGI(TAG, "OTA START: %lu bytes expected", pkt->total_size);
        send_response(OTA_CMD_ACK, 0);
        break;

    case OTA_CMD_DATA:
        if (!s_ota_in_progress) return;

        if (pkt->seq != s_expected_seq) {
            ESP_LOGW(TAG, "Seq mismatch: got %lu, want %lu",
                     pkt->seq, s_expected_seq);
            send_response(OTA_CMD_NACK, s_expected_seq);
            return;
        }

        uint16_t crc = esp_crc16_le(0, pkt->data, pkt->data_len);
        if (crc != pkt->crc16) {
            ESP_LOGW(TAG, "CRC error on seq %lu", pkt->seq);
            send_response(OTA_CMD_NACK, pkt->seq);
            return;
        }

        esp_ota_write(s_ota_handle, pkt->data, pkt->data_len);
        s_expected_seq++;
        send_response(OTA_CMD_ACK, pkt->seq);

        if (s_expected_seq % 50 == 0)
            ESP_LOGI(TAG, "Written %lu chunks...", s_expected_seq);
        break;

    case OTA_CMD_END:
        if (!s_ota_in_progress) return;

        esp_err_t err = esp_ota_end(s_ota_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_end failed: %s", esp_err_to_name(err));
            send_response(OTA_CMD_NACK, pkt->seq);
            return;
        }

        esp_ota_set_boot_partition(s_ota_part);
        ESP_LOGI(TAG, "OTA complete! Rebooting in 1s...");
        send_response(OTA_CMD_ACK, pkt->seq);

        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();
        break;
    }
}

void ota_receiver_init(void) {
    esp_now_register_recv_cb(ota_rx_on_recv);
    ESP_LOGI(TAG, "OTA receiver ready");
}