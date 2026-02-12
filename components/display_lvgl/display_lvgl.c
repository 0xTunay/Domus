#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/spi_master.h"

#include "display_lvgl.h"

static const char *TAG = "display_lvgl";

#define PIN_NUM_MOSI 4
#define PIN_NUM_MISO -1
#define PIN_NUM_CLK 6
#define PIN_NUM_CS 7
#define PIN_NUM_DC 8
#define PIN_NUM_RST 9
#define PIN_NUM_BCKL 10

#define SPI_HOST_USED SPI2_HOST

esp_err_t DisplayLvglInit(void)
{
    ESP_LOGI(TAG, "Initializing SPI bus for LCD...");

    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 320*240*2 + 8
    };

    esp_err_t ret = spi_bus_initialize(SPI_HOST_USED, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "SPI bus initialized successfully");
    return ESP_OK;
}

esp_err_t DisplayLvglUpdateTemp(float temperature)
{
    ESP_LOGI(TAG, "Temperature: %.2f", temperature);
    return ESP_OK;
}

void DisplayLvglTask(void *pvParameters)
{
    while (1) {
        DisplayLvglUpdateTemp(25.0f);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
