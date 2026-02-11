#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/spi_master.h"

#include "display_lvgl.h"

static const char *TAG = "display_lvgl";

/* Настройка пинов под SPI */
#define PIN_NUM_MOSI 4
#define PIN_NUM_MISO -1    // LCD часто только MOSI, MISO не нужен
#define PIN_NUM_CLK 6
#define PIN_NUM_CS 7       // Chip select
#define PIN_NUM_DC 8       // Data/command
#define PIN_NUM_RST 9      // Reset (если есть)
#define PIN_NUM_BCKL 10    // Подсветка (если есть)

/* SPI host, используем VSPI на ESP32 */
#define SPI_HOST_USED SPI2_HOST

esp_err_t DisplayLvglInit(void)
{
    ESP_LOGI(TAG, "Initializing SPI bus for LCD...");

    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO, // если LCD не читает — можно -1
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 320*240*2 + 8
    };

    /* Инициализация SPI */
    esp_err_t ret = spi_bus_initialize(SPI_HOST_USED, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "SPI bus initialized successfully");
    // Здесь можно добавить init драйвера LCD, например ili9341_init() и включение подсветки
    return ESP_OK;
}

esp_err_t DisplayLvglUpdateTemp(float temperature)
{
    // Просто заглушка, потом сюда можно обновление на экране
    ESP_LOGI(TAG, "Temperature: %.2f", temperature);
    return ESP_OK;
}

void DisplayLvglTask(void *pvParameters)
{
    while (1) {
        // Заглушка: например, обновление температуры каждые 1с
        DisplayLvglUpdateTemp(25.0f);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
