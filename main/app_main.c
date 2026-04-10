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
#include "Sensors_types.h"
#include "receive_queue.h"

TaskHandle_t DisplayLvglTaskHandle = NULL;
TaskHandle_t SensorTaskHandle = NULL;
SemaphoreHandle_t SensorSemaphoreHandle = NULL;

static const char *TAG = "main";


void vSensorTask(void *pvParameter) { /* implementation of data reception from ESP at present  */
    SensorData_t *data = (SensorData_t *)pvParameter;
    int error_count = 0;
    const int max_errors = 5;

    while (error_count < max_errors) {
     

        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    ESP_LOGE(TAG, "Sensor fatal error. Task deleting...");
    vTaskDelete(NULL);
}

void ControlTask(void *pvParameter) {
    SensorData_t data = {0};

    data.humidity = 0;
    data.temperature = 0;
    data.humidity = 0;

    char payload[64];
    const TickType_t xTicksToWait = pdMS_TO_TICKS(5000);
    bool SensorEnable = true;

    uint32_t last_temp = UINT32_MAX;
    uint32_t last_hum  = UINT32_MAX;
    uint32_t last_press = UINT32_MAX;

    int same_temp_count = 0;
    int same_hum_count  = 0;
    int same_press_count = 0;

    const int FORCE_SEND_COUNT = 5;

    while (SensorEnable) {

        receive_queue_humidity(&data);
        receive_queue_pressure(&data);
        receive_queue_temperature(&data);
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
            switch (st){
            case eRunning:   ESP_LOGI(TAG, "SensorTask: Running");   break;
            case eSuspended: ESP_LOGI(TAG, "SensorTask: Suspended"); break;
            case eReady:     ESP_LOGI(TAG, "SensorTask: Ready");     break;
            case eBlocked:   ESP_LOGI(TAG, "SensorTask: Blocked");   break;
            case eDeleted:   ESP_LOGI(TAG, "SensorTask: Deleted");   break;
            default:         ESP_LOGI(TAG, "SensorTask: Unknown");   break;
            }
        } else {
            ESP_LOGW(TAG, "SensorTaskHandle is NULL");
        }

        if (TemperatureQueueHandle != NULL) {
            ESP_LOGI(TAG, "TempQueue:  %u/%u",
                     (unsigned)uxQueueMessagesWaiting(TemperatureQueueHandle),
                     (unsigned)uxQueueSpacesAvailable(TemperatureQueueHandle)
                         + (unsigned)uxQueueMessagesWaiting(TemperatureQueueHandle));
        }

        if (HumidityQueueHandle != NULL) {
            ESP_LOGI(TAG, "HumQueue:   %u/%u",
                     (unsigned)uxQueueMessagesWaiting(HumidityQueueHandle),
                     (unsigned)uxQueueSpacesAvailable(HumidityQueueHandle)
                         + (unsigned)uxQueueMessagesWaiting(HumidityQueueHandle));
        }

        if (PressureQueueHandle != NULL) {
            ESP_LOGI(TAG, "PressQueue: %u/%u",
                     (unsigned)uxQueueMessagesWaiting(PressureQueueHandle),
                     (unsigned)uxQueueSpacesAvailable(PressureQueueHandle)
                         + (unsigned)uxQueueMessagesWaiting(PressureQueueHandle));
        }
        /* free heap for debugging */
        ESP_LOGI(TAG, "Free heap: %lu bytes",
                 (unsigned long)esp_get_free_heap_size());

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    LedInit();
    LedOFF();

    receive_queue_init();

    /* NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_sta();
   // wifi_wait_connected();
  //  esp_wifi_set_ps(WIFI_PS_NONE);

    mqtt_app_start();
    espnow_master_init();

    if (xTaskCreate(ControlTask, "ControlTask", 4096, NULL, 6, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create ControlTask");
    }

    if (xTaskCreate(vTaskMonitor, "vTaskMonitor",
                    configMINIMAL_STACK_SIZE * 2, NULL, 3, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create vTaskMonitor");
    }

    if (xTaskCreate(LedBlink, "LedBlink",
                    configMINIMAL_STACK_SIZE * 2, NULL, 3, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LedBlink");
    }
}