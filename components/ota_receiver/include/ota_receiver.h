#ifndef OTA_RECEIVER_H
#define OTA_RECEIVER_H

#include <stdint.h>

#define CHUNK_SIZE 200

typedef enum {
    OTA_CMD_START = 0x01,
    OTA_CMD_DATA  = 0x02,
    OTA_CMD_END   = 0x03,
    OTA_CMD_ACK   = 0x04,
    OTA_CMD_NACK  = 0x05,
} ota_cmd_t;

typedef struct {
    uint8_t  cmd;
    uint32_t seq;
    uint32_t total_size;
    uint16_t data_len;
    uint16_t crc16;
    uint8_t  data[CHUNK_SIZE];
} __attribute__((packed)) ota_pkt_t;

void ota_receiver_init(void);

#endif OTA_RECEIVER_H