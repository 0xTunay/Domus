#include <stdio.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <inttypes.h>
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "bme280.h"
#include "driver/gpio.h"
// #include "display_lvgl.h"
#include "app_main.h"
#include "BME280.h"
#include "sensors_types.h"
#include "espnow_handler.h"

TaskHandle_t DisplayLvglTaskHandle = NULL;
QueueHandle_t SensorQueueHandle = NULL;
TaskHandle_t SensorTaskHandle = NULL;
SemaphoreHandle_t SensorSemaphoreHandle = NULL;

static const char *TAG = "main";

static void system_init(void) {
    ESP_LOGI(TAG, "System init start");

    /* === NVS === */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    /* === I2C === */
    ESP_LOGI(TAG, "Init I2C...");
    i2c_bus_init();
    if (i2c_bus == NULL) {
        ESP_LOGE(TAG, "I2C init FAILED");
        abort();
    }

    /* === BME280 === */
    ESP_LOGI(TAG, "Init BME280...");
    bme280_init();
    if (bme280 == NULL) {
        ESP_LOGE(TAG, "BME280 init FAILED");
        abort();
    }

    /* === ESP-NOW === */
    ESP_LOGI(TAG, "Init ESP-NOW...");
    espnow_handler_init();

    ESP_LOGI(TAG, "System init done");
}

/* ========================================================= */

void vSensorTask(void *pvParameter)
{
    SensorData_t data;

    while (1) {
        if (bme280_read_temperature(bme280, &data.temperature) != ESP_OK ||
            bme280_read_humidity(bme280, &data.humidity) != ESP_OK ||
            bme280_read_pressure(bme280, &data.pressure) != ESP_OK) {

            ESP_LOGE(TAG, "BME280 read error");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        ESP_LOGI(TAG,
                 "T=%.1f C, H=%.1f %%, P=%.1f hPa",
                 data.temperature,
                 data.humidity,
                 data.pressure / 100.0f);

        if (xQueueSend(SensorQueueHandle, &data, 0) != pdPASS) {
            ESP_LOGW(TAG, "Queue full, dropping sample");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void vControlTask(void *pvParameter) {
    SensorData_t data;

    while (1) {
        if (xQueueReceive(SensorQueueHandle, &data, portMAX_DELAY) == pdTRUE) {

            ESP_LOGI(TAG,
                     "TX: T=%.1f C, H=%.1f %%, P=%.1f hPa",
                     data.temperature,
                     data.humidity,
                     data.pressure / 10.0f);

            espnow_send_telemetry(&data);
        }
    }
}


void vMonitorTask(void *pvParameter) {
    while (1) {
        UBaseType_t used = uxQueueMessagesWaiting(SensorQueueHandle);

        ESP_LOGI(TAG, "Queue load: %d", (int)used);

        if (SensorTaskHandle != NULL) {
            eTaskState st = eTaskGetState(SensorTaskHandle);
            ESP_LOGI(TAG, "SensorTask state: %d", st);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/* ========================================================= */

void app_main(void) {
    ESP_LOGI(TAG, "Boot");

    system_init();

    /* === QUEUE === */
    SensorQueueHandle = xQueueCreate(ITEM_SIZE, sizeof(SensorData_t));
    if (SensorQueueHandle == NULL) {
        ESP_LOGE(TAG, "Queue create failed");
        abort();
    }

    /* === TASKS === */
    BaseType_t res;

    res = xTaskCreate(vSensorTask,
                      "SensorTask",
                      4096,
                      NULL,
                      5,
                      &SensorTaskHandle);
    if (res != pdPASS) {
        ESP_LOGE(TAG, "SensorTask create failed");
        abort();
    }

    res = xTaskCreate(vControlTask,
                      "ControlTask",
                      4096,
                      NULL,
                      6,
                      NULL);
    if (res != pdPASS) {
        ESP_LOGE(TAG, "ControlTask create failed");
        abort();
    }

    res = xTaskCreate(vMonitorTask,
                      "MonitorTask",
                      2048,
                      NULL,
                      3,
                      NULL);
    if (res != pdPASS) {
        ESP_LOGE(TAG, "MonitorTask create failed");
        abort();
    }

    ESP_LOGI(TAG, "System started");
}