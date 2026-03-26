#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <inttypes.h>
#include "freertos/semphr.h"
#include "nvs_flash.h"

#include "app_main.h"
// #include "display_lvgl.h"
#include "mosquitto.h"
#include "mqtt_client.h"
#include "wifi_manager.h"
#include "espnow_handler.h"

TaskHandle_t DisplayLvglTaskHandle = NULL;
QueueHandle_t SensorQueueHandle = NULL;
TaskHandle_t SensorTaskHandle = NULL;
SemaphoreHandle_t SensorSemaphoreHandle = NULL;
QueueHandle_t HumidityQueueHandle = NULL;

static const char *TAG = "main";


void vSensorTask(void *pvParameter) { /* implementation of data reception from ESP at present  */
    int error_count = 0;
    const int max_errors = 5;

    while (error_count < max_errors) {
     

        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    ESP_LOGE(TAG, "Sensor fatal error. Task deleting...");
    vTaskDelete(NULL);
}

void ControlTask(void *pvParameter) {
    uint32_t temp_x10 = 0;
    uint32_t hum_x10 = 0;
    char payload[64];
    const TickType_t xTicksToWait = pdMS_TO_TICKS(5000);
    bool SensorEnable = true;

    uint32_t last_temp = UINT32_MAX;
    uint32_t last_hum  = UINT32_MAX;
    int same_temp_count = 0;
    int same_hum_count  = 0;
    const int FORCE_SEND_COUNT = 5;

    while (SensorEnable) {
        if (xQueueReceive(SensorQueueHandle, &temp_x10, xTicksToWait) == pdTRUE) {
            bool changed = (temp_x10 != last_temp);

            if (changed) {
                same_temp_count = 0;
            } else {
                same_temp_count++;
            }

            bool should_send = changed || (same_temp_count >= FORCE_SEND_COUNT);

            if (should_send)
            {
                if (same_temp_count >= FORCE_SEND_COUNT)
                {
                    same_temp_count = 0;
                }

                last_temp = temp_x10;
                float temperature = temp_x10 / 10.0f;
                snprintf(payload, sizeof(payload), "%.1f", temperature);

                ESP_LOGI(TAG, "Received temp: %" PRIu32 " (%.1f C) [%s]",
                         temp_x10, temperature, changed ? "changed" : "force");

                if (s_client != NULL && xSemaphoreTake(s_mqtt_mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
                {
                    int msg_id = esp_mqtt_client_publish(s_client, "sensors/temperature", payload, 0, 1, 0);
                    ESP_LOGI(TAG, "Published temp: %s, msg_id=%d", payload, msg_id);
                    xSemaphoreGive(s_mqtt_mutex);
                }
                else
                {
                    ESP_LOGW(TAG, "MQTT not ready or mutex timeout");
                }
            }
            else
            {
                ESP_LOGI(TAG, "Temp unchanged (%.1f C), skip [%d/%d]",
                         temp_x10 / 10.0f, same_temp_count, FORCE_SEND_COUNT);
            }
        }

        if (xQueueReceive(HumidityQueueHandle, &hum_x10, xTicksToWait) == pdTRUE)
        {
            bool changed = (hum_x10 != last_hum);

            if (changed)
            {
                same_hum_count = 0;
            }
            else
            {
                same_hum_count++;
            }

            bool should_send = changed || (same_hum_count >= FORCE_SEND_COUNT);

            if (should_send)
            {
                if (same_hum_count >= FORCE_SEND_COUNT)
                {
                    same_hum_count = 0;
                }

                last_hum = hum_x10;
                float humidity = hum_x10 / 10.0f;
                snprintf(payload, sizeof(payload), "%.1f", humidity);

                ESP_LOGI(TAG, "Received hum: %" PRIu32 " (%.1f %%) [%s]",
                         hum_x10, humidity, changed ? "changed" : "force");

                if (s_client != NULL && xSemaphoreTake(s_mqtt_mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
                {
                    int msg_id = esp_mqtt_client_publish(s_client, "sensors/humidity", payload, 0, 1, 0);
                    ESP_LOGI(TAG, "Published hum: %s, msg_id=%d", payload, msg_id);
                    xSemaphoreGive(s_mqtt_mutex);
                }
                else
                {
                    ESP_LOGW(TAG, "MQTT not ready or mutex timeout");
                }
            }
            else
            {
                ESP_LOGI(TAG, "Hum unchanged (%.1f %%), skip [%d/%d]",
                         hum_x10 / 10.0f, same_hum_count, FORCE_SEND_COUNT);
            }
        }
    }

    vTaskDelete(NULL);
}
void vTaskMonitor(void *pvParameter)
{
    for (;;)
    {
        if (SensorTaskHandle != NULL)
        {
            eTaskState st = eTaskGetState(SensorTaskHandle);
            switch (st)
            {
            case eRunning:
                ESP_LOGI(TAG, "Task Running");
                break;
            case eSuspended:
                ESP_LOGI(TAG, "Task Suspended");
                break;
            case eReady:
                ESP_LOGI(TAG, "Task Ready");
                break;
            case eBlocked:
                ESP_LOGI(TAG, "Task Blocked");
                break;
            case eDeleted:
                ESP_LOGI(TAG, "Task Deleted");
                break;
            default:
                ESP_LOGI(TAG, "Task Default");
                break;
            }
        }
        else
        {
            ESP_LOGW(TAG, "Sensor null");
        }

        UBaseType_t used = uxQueueMessagesWaiting(SensorQueueHandle);
        ESP_LOGI(TAG, "queue item size %d", (unsigned)used);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    LedInit();
    LedOFF();
    ESP_LOGI(TAG, "Initializing the Sensor");

    /*====================== WIFI INIT ======================*/
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    if (CONFIG_LOG_MAXIMUM_LEVEL > CONFIG_LOG_DEFAULT_LEVEL)
    {
        esp_log_level_set("wifi", CONFIG_LOG_MAXIMUM_LEVEL);
    }

        SensorQueueHandle = xQueueCreate(ITEM_SIZE, sizeof(uint32_t));
    if (SensorQueueHandle == NULL)
    {
        ESP_LOGE(TAG, "Failed to create SensorTaskQueue");
        return;
    }

    HumidityQueueHandle = xQueueCreate(ITEM_SIZE, sizeof(uint32_t));
    if (HumidityQueueHandle == NULL)
    {
        ESP_LOGE(TAG, "Failed to create HumidityQueue");
        return;
    }
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    /*====================== WIFI INIT ======================*/

    /*=== MQTT INIT === */
    mqtt_app_start();
    /*=== MQTT INIT === */


    if (xTaskCreate(vSensorTask,
                    "vSensorTask",
                    configMINIMAL_STACK_SIZE * 2,
                    NULL,
                    4,
                    &SensorTaskHandle) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create vSensorTask");
    }

    if (xTaskCreate(ControlTask,
                    "ControlTask",
                    4096,
                    NULL,
                    6,
                    NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create ControlTask");
    }

    if (xTaskCreate(vTaskMonitor,
                    "vTaskMonitor",
                    configMINIMAL_STACK_SIZE * 2,
                    NULL,
                    3,
                    NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create vTaskMonitor");
    }

    if (xTaskCreate(LedBlink,
                    "LedBlink",
                    configMINIMAL_STACK_SIZE * 2,
                    NULL,
                    3,
                    NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create LedBlink");
    }
}