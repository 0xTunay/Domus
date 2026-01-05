#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "freertos/semphr.h"

#include "driver/spi_master.h"

#include "display_lvgl.h"

static const char *TAG = "display_lvgl";

#define MISO 4
#define MOSI 5
#define SCK 6
#define CLK 7
#define LCD 32

esp_err_t DisplayLvglInit(void) {
    spi_bus_config_t buscfg = {
        .mosi_io_num = SCK,
        .miso_io_num = MISO,
        .sclk_io_num = MOSI,
        .mosi_io_num = CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD, &buscfg, SPI_DMA_CH_AUTO));
    return 0;
}
esp_err_t DisplayLvglUpdateTemp(float temperature) {

    return 0;
}
void DisplayLvglTask(void *pvParameters) {
    for (;;) {

    }
}