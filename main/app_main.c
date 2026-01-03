#include <stdio.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <inttypes.h>

#include "app_main.h"
#include "dht.h"

#define ITEM_SIZE 5

#define DHT_GPIO GPIO_NUM_19


QueueHandle_t SensorTaskQueueHandle = NULL;
TaskHandle_t SensorTaskHandle = NULL;

static const char *TAG = "main";


void vSensorTask(void *pvParameter) {
    float humidity = 0, temperature = 0;
    BaseType_t xReturn;

    for (;;) {
        esp_err_t ret = dht_read_float_data(DHT_TYPE_AM2301, DHT_GPIO, &humidity, &temperature);

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Read DHT: Temp %.1f, Hum %.1f", temperature, humidity);

            uint32_t data_to_send = (uint32_t)temperature;
            xReturn = xQueueOverwrite((void*)SensorTaskHandle, (void*)&data_to_send);
            if (xReturn == pdTRUE) {
                ESP_LOGI(TAG, "Item Send: %" PRIu32, data_to_send);

            }
        } else {
            ESP_LOGE(TAG, "Could not read data from sensor: %s", esp_err_to_name(ret));
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
/*
 * https://docs.espressif.com/projects/esp-techpedia/en/latest/esp-friends/get-started/basic-concepts/common-freertos-api/queue-management.html#receiving-data
 */
void ControlTask(void *pvParameter) {
    BaseType_t xReturn;
    uint32_t receiver = 0;

    for (;;) {

        xReturn = xQueueReceive((void*)SensorTaskQueueHandle, (void*)&receiver, portMAX_DELAY);
        if (xReturn == pdTRUE) {
            ESP_LOGI(TAG, "Item Receive: %" PRIu32, receiver);
        } else {
            ESP_LOGE(TAG, "Item Receive Error");
        }
        if (receiver >= 22) {
            LedON();
        } else {
            ledBlink();
        }
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

        UBaseType_t used = uxQueueMessagesWaiting(SensorTaskQueueHandle);
        ESP_LOGI(TAG,"queue item size %d", (unsigned)used);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void app_main(void)
{
    ledInit();
    LedOFF();
    ESP_LOGI(TAG, "Initializing the Sensor");

    SensorTaskQueueHandle = xQueueCreate(ITEM_SIZE, sizeof(uint32_t));
    if (SensorTaskQueueHandle == NULL) {
        ESP_LOGE(TAG, "Failed to create SensorTaskQueue");
        return;
    }

    if (xTaskCreate(vSensorTask,
                "vSensorTask",
                configMINIMAL_STACK_SIZE * 2,
                NULL,
                5,
                &SensorTaskHandle) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create vSensorTask");
    }

    if (xTaskCreate(ControlTask,
                "ControlTask",
                2048,
                NULL,
                5,
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
// Create the best solution for blink
//
//*//