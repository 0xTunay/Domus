#include <stdio.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <inttypes.h>
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "i2c_bus.h"
#include "bme280.h"

// #include "display_lvgl.h"
#include "app_main.h"
#include "mosquitto.h"
#include "wifi_manager.h"
#include "BME280.h"

#define DHT_GPIO 23
TaskHandle_t DisplayLvglTaskHandle = NULL;
QueueHandle_t SensorQueueHandle = NULL;
TaskHandle_t SensorTaskHandle = NULL;
SemaphoreHandle_t SensorSemaphoreHandle = NULL;

static const char *TAG = "main";


void vSensorTask(void *pvParameter) {
    float temperature = 0.0, humidity = 0.0, pressure = 0.0;
    int error_count = 0;
    const int max_errors = 5;

    while (error_count < max_errors) {
        esp_err_t ret = ESP_OK;
        ret |= bme280_read_temperature(bme280, &temperature);
        ret |= bme280_read_humidity(bme280, &humidity);
        ret |= bme280_read_pressure(bme280, &pressure);

        if (ret == ESP_OK) {
            error_count = 0;
            uint32_t temp_x10 = (uint32_t)(temperature * 10.0f);

            ESP_LOGI(TAG, "BME280: T=%.1f C, H=%.1f %%, P=%.1f hPa",
                            temperature,humidity, pressure / 100.0f);

            if (xQueueSend(SensorQueueHandle, &temp_x10, 0) != pdTRUE) {
                ESP_LOGW(TAG, "Queue full, skip ");
            } else {
                error_count++;
                ESP_LOGE(TAG, "BME280 read failed (%d/%d)", error_count, max_errors);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    ESP_LOGE(TAG, "BME280 fatal error. Task deleting...");
    vTaskDelete(NULL);
}
void ControlTask(void *pvParameter) {
    uint32_t temp_x10 = 0;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(5000);
    bool SensorEnable = true;
    while (SensorEnable) {
        if (xQueueReceive(SensorQueueHandle, &temp_x10, xTicksToWait) == pdTRUE) {

            ESP_LOGI(TAG, "Received temp: %" PRIu32 " (%.1f C)",
                     temp_x10, temp_x10 / 10.0f);

            if (temp_x10 >= 280) {
                BlinkEnable = false;
                LedON();
            } else {
                BlinkEnable = true;
                LedOFF();
            }
        } else {
            ESP_LOGW(TAG, "Error: data from sensor not receined ");
            SensorEnable = false;
        }

       /* vTaskDelay(pdMS_TO_TICKS(10)); */
    }
    vTaskDelete(NULL);

}

void vTaskMonitor(void *pvParameter) {

    for (;;) {
        if (SensorTaskHandle != NULL) {
                eTaskState st = eTaskGetState(SensorTaskHandle);
            switch (st) {
                case eRunning: ESP_LOGI(TAG,"Task Running\n"); break;
                case eSuspended: ESP_LOGI(TAG,"Task Suspended\n"); break;
                case eReady: ESP_LOGI(TAG,"Task Ready\n"); break;
                case eBlocked: ESP_LOGI(TAG,"Task Blocked\n"); break;
                case eDeleted: ESP_LOGI(TAG,"Task Deleted\n"); break;
                default: ESP_LOGI(TAG,"Task Default\n"); break;
            }
        } else {
            ESP_LOGW(TAG, "Sensor null");
        }

        UBaseType_t used = uxQueueMessagesWaiting(SensorQueueHandle);
        ESP_LOGI(TAG,"queue item size %d", (unsigned)used);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    LedInit();
    LedOFF();
    ESP_LOGI(TAG, "Initializing the Sensor");
   //  ESP_ERROR_CHECK(DisplayLvglInit());
    /*====================== WIFI INIT ======================*/
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    if (CONFIG_LOG_MAXIMUM_LEVEL > CONFIG_LOG_DEFAULT_LEVEL) {
        /* If you only want to open more logs in the wifi module, you need to make the max level greater than the default level,
         * and call esp_log_level_set() before esp_wifi_init() to improve the log level of the wifi module. */
        esp_log_level_set("wifi", CONFIG_LOG_MAXIMUM_LEVEL);
    }

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    /*====================== WIFI INIT ======================*/

    /*=== MQTT INIT === */
    mqtt_app_start();
    /*=== MQTT INIT === */

    /*=== BME280 INIT === */
    bme280_init();
    /*=== BME280 INIT === */

    SensorQueueHandle = xQueueCreate(ITEM_SIZE, sizeof(uint32_t));
    if (SensorQueueHandle == NULL) {
        ESP_LOGE(TAG, "Failed to create SensorTaskQueue");
        return;
    }
// if (xTaskCreate(
//             DisplayLvglTask,
//             "DisplayLvglTask",
//             4096,
//             NULL,
//             5,
//             NULL) != pdPASS) {
//         ESP_LOGE(TAG, "Failed to create DisplayLvglTask");
//     }
    if (xTaskCreate(vSensorTask,
                "vSensorTask",
                configMINIMAL_STACK_SIZE * 2,
                NULL,
                4,
                &SensorTaskHandle) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create vSensorTask");
    }

    if (xTaskCreate(ControlTask,
                "ControlTask",
                4096,
                NULL,
                6,
                NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create ControlTask");
    }

    if (xTaskCreate(vTaskMonitor,
            "vTaskMonitor",
            configMINIMAL_STACK_SIZE * 2,
            NULL,
            3,
            NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create vTaskMonitor");
            }
    if (xTaskCreate(LedBlink,
                "LedBlink",
                configMINIMAL_STACK_SIZE * 2,
                NULL,
                3,
                NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LedBlink");
                }
}