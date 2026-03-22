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

TaskHandle_t DisplayLvglTaskHandle = NULL;
QueueHandle_t SensorQueueHandle = NULL;
TaskHandle_t SensorTaskHandle = NULL;
SemaphoreHandle_t SensorSemaphoreHandle = NULL;

static const char *TAG = "main";


void vSensorTask(void *pvParameter) {
    int error_count = 0;
    const int max_errors = 5;

    while (error_count < max_errors) {
        SensorData_t data = {0};
        esp_err_t ret = ESP_OK;
        ret |= bme280_read_temperature(bme280, &data.temperature);
        ret |= bme280_read_humidity(bme280, &data.humidity);
        ret |= bme280_read_pressure(bme280, &data.pressure);

        if (ret == ESP_OK) {
            error_count = 0;

            ESP_LOGI(TAG, "BME280: T=%.1f C, H=%.1f %%, P=%.1f hPa",
                    data.temperature,
                    data.humidity,
                    data.pressure / 100.0f);
            if (xQueueSend(SensorQueueHandle, &data, pdMS_TO_TICKS(1000)) != pdPASS) {
                error_count++;
                ESP_LOGE(TAG, "BME280 read failed (%d/%d)",
                         error_count, 
                         max_errors);
            } else {
                ESP_LOGI(TAG, "Data sent to queue successfully");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    ESP_LOGE(TAG, "BME280 fatal error. Task deleting...");
    vTaskDelete(NULL);
}
void ControlTask(void *pvParameter) {
    // function to send data to esp gateway with esp-now protocol
    SensorData_t data;
   // char payload[128];
    const TickType_t xTicksToWait = pdMS_TO_TICKS(5000);
    bool SensorEnable = true;

    while(SensorEnable){
        if (xQueueReceive(SensorQueueHandle, &data, xTicksToWait) == pdTRUE) {
            ESP_LOGI(TAG, "Received sensor data: T=%.1f C, H=%.1f %%, P=%.1f hPa", 
                     data.temperature,
                     data.humidity,
                     data.pressure / 100.0f);
            // send to esp gateway with esp now
        } else {
            ESP_LOGW(TAG, "No temperature received within timeout");
        }  
        vTaskDelay(pdMS_TO_TICKS(1000));
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
        ESP_LOGI(TAG,"queue item size %d",
                 (unsigned)used);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    LedInit();
    LedOFF();
    ESP_LOGI(TAG, "Initializing the Sensor");
    i2c_bus_init();
    /*=== BME280 I NIT === */
    bme280_init();
    /*=== BME280 INIT === */

    SensorQueueHandle = xQueueCreate(ITEM_SIZE, sizeof(SensorData_t));
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
                4096,
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
