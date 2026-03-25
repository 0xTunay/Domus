#ifndef ESPNOW_HANDLER_H
#define ESPNOW_HANDLER_H

#include <stdint.h>
#include <string.h>
#include "esp_now.h"
#include "sensors_types.h"

typedef enum {
    PKT_TELEMETRY = 0x01,
    PKT_OTA_BEGIN = 0x10,
    PKT_OTA_DATA  = 0x11,
    PKT_OTA_END   = 0x12,
} espnow_pkt_type_t;

typedef struct {
    espnow_pkt_type_t type;
    uint8_t           seq;
    uint16_t          crc;
    SensorData_t      payload;
} __attribute__((packed)) espnow_packet_t;

void espnow_handler_init(void);
void espnow_send_telemetry(const SensorData_t *data);

#endif // ESPNOW_HANDLER_H