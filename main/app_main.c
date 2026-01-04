#include <stdio.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <inttypes.h>

#include "app_main.h"
#include "dht.h"



QueueHandle_t SensorQueueHandle = NULL;
TaskHandle_t SensorTaskHandle = NULL;

static const char *TAG = "main";


void vSensorTask(void *pvParameter) {
    float humidity = 0.0f, temperature = 0.0f;

    for (;;) {
        esp_err_t ret = dht_read_float_data(DHT_TYPE_AM2301, DHT_GPIO, &humidity, &temperature);

        if (ret == ESP_OK) {
            uint32_t temp_x10 = (uint32_t)(temperature * 10.0f);

            ESP_LOGI(TAG, "Read DHT: Temp %.1f, Hum %.1f", temperature, humidity);

           /*  uint16_t data_to_send = (uint32_t)temperature; */
                if (uxQueueMessagesWaiting(SensorQueueHandle) == ITEM_SIZE) {
                uint32_t dummy = 0;
                xQueueReceive(SensorQueueHandle, &dummy, 0);
                ESP_LOGI(TAG, "Queue restart");
            }

            if (xQueueSend(SensorQueueHandle, &temp_x10, 0) != pdTRUE) {
                ESP_LOGW(TAG, "Restart queue");
            }
        } else {
            ESP_LOGE(TAG, "DHT read failed: %s", esp_err_to_name(ret));
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
void ControlTask(void *pvParameter) {
    /* BaseType_t xReturn; */
    uint32_t temp_x10 = 0;

    for (;;) {
        if (xQueueReceive(SensorQueueHandle, &temp_x10, portMAX_DELAY) == pdTRUE) {

            ESP_LOGI(TAG, "Received temp: %" PRIu32 " (%.1f C)",
                     temp_x10, temp_x10 / 10.0f);

            if (temp_x10 >= 280) {
                BlinkEnable = false;
                LedON();
            } else {
                BlinkEnable = true;
                LedOFF();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
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

    SensorQueueHandle = xQueueCreate(ITEM_SIZE, sizeof(uint32_t));
    if (SensorQueueHandle == NULL) {
        ESP_LOGE(TAG, "Failed to create SensorTaskQueue");
        return;
    }

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

// *
// ===============================================================================================================
//                                        ESPRESSIF DOCUMANTION
//
//  https://docs.espressif.com/projects/esp-techpedia/en/latest/esp-friends/get-started/basic-concepts/common-freertos-api/queue-management.html#receiving-data
//
//
// ===============================================================================================================
// */

//*
// TODO
// add mutex
//*//