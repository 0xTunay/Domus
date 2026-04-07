#include <stdio.h>
#include <inttypes.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "receive_queue.h"
#include "mosquitto.h"
#include "Sensors_types.h"

static const char *TAG = "[receive_queue]";

QueueHandle_t TemperatureQueueHandle = NULL;
QueueHandle_t HumidityQueueHandle    = NULL;
QueueHandle_t PressureQueueHandle    = NULL;


void receive_queue_init(void)
{
    TemperatureQueueHandle = xQueueCreate(QUEUE_SIZE, sizeof(float));
    if (TemperatureQueueHandle == NULL) {
        ESP_LOGE(TAG, "Failed to create TemperatureQueue");
    }

    HumidityQueueHandle = xQueueCreate(QUEUE_SIZE, sizeof(float));
    if (HumidityQueueHandle == NULL) {
        ESP_LOGE(TAG, "Failed to create HumidityQueue");
    }

    PressureQueueHandle = xQueueCreate(QUEUE_SIZE, sizeof(float));
    if (PressureQueueHandle == NULL) {
        ESP_LOGE(TAG, "Failed to create PressureQueue");
    }

    ESP_LOGI(TAG, "All queues initialized");
}


static void mqtt_publish_float(const char *topic, float value, const char *label)
{
    char payload[32];
    snprintf(payload, sizeof(payload), "%.1f", value);

    if (s_client != NULL &&
        xSemaphoreTake(s_mqtt_mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        int msg_id = esp_mqtt_client_publish(s_client, topic, payload, 0, 1, 0);
        ESP_LOGI(TAG, "Published %s: %s, msg_id=%d", label, payload, msg_id);
        xSemaphoreGive(s_mqtt_mutex);
    } else {
        ESP_LOGW(TAG, "MQTT not ready or mutex timeout (topic: %s)", topic);
    }
}

void receive_queue_temperature(SensorData_t *data)
{
    static float last_temp = -9999.0f; /* obviously impossible temperature */
    static int same_temp_count = 0;

    const TickType_t xTicksToWait   = pdMS_TO_TICKS(5000);
    const int FORCE_SEND_COUNT = 5;

    if (xQueueReceive(TemperatureQueueHandle, &data->temperature, xTicksToWait) != pdTRUE) {
        ESP_LOGW(TAG, "Temperature queue timeout");
        return;
    }

    bool changed = (data->temperature != last_temp);

    if (changed) {
        same_temp_count = 0;
    } else {
        same_temp_count++;
    }

    bool should_send = changed || (same_temp_count >= FORCE_SEND_COUNT);

    if (!should_send) {
        ESP_LOGI(TAG, "Temp unchanged (%.1f C), skip [%d/%d]",
                 data->temperature, same_temp_count, FORCE_SEND_COUNT);
        return;
    }

    if (same_temp_count >= FORCE_SEND_COUNT) {
        same_temp_count = 0;
    }

    last_temp = data->temperature;

    ESP_LOGI(TAG, "Temp: %.1f C [%s]",
             data->temperature, changed ? "changed" : "force");

    mqtt_publish_float("sensors/temperature", data->temperature,
                       "temperature");
}


void receive_queue_humidity(SensorData_t *data)
{
    static float    last_hum       = -9999.0f;
    static int      same_hum_count = 0;

    const TickType_t xTicksToWait  = pdMS_TO_TICKS(5000);
    const int        FORCE_SEND_COUNT = 5;

    if (xQueueReceive(HumidityQueueHandle, &data->humidity, xTicksToWait) != pdTRUE) {
        ESP_LOGW(TAG, "Humidity queue timeout");
        return;
    }

    bool changed = (data->humidity != last_hum);

    if (changed) {
        same_hum_count = 0;
    } else {
        same_hum_count++;
    }

    bool should_send = changed || (same_hum_count >= FORCE_SEND_COUNT);

    if (!should_send) {
        ESP_LOGI(TAG, "Hum unchanged (%.1f %%), skip [%d/%d]",
                 data->humidity, same_hum_count, FORCE_SEND_COUNT);
        return;
    }

    if (same_hum_count >= FORCE_SEND_COUNT) {
        same_hum_count = 0;
    }

    last_hum = data->humidity;

    ESP_LOGI(TAG, "Hum: %.1f %% [%s]",
             data->humidity, changed ? "changed" : "force");

    mqtt_publish_float("sensors/humidity", data->humidity, "humidity");
}

void receive_queue_pressure(SensorData_t *data)
{
    static float    last_press       = -9999.0f;
    static int      same_press_count = 0;

    const TickType_t xTicksToWait    = pdMS_TO_TICKS(5000);
    const int        FORCE_SEND_COUNT = 5;

    if (xQueueReceive(PressureQueueHandle, &data->pressure, xTicksToWait) != pdTRUE) {
        ESP_LOGW(TAG, "Pressure queue timeout");
        return;
    }

    bool changed = (data->pressure != last_press);

    if (changed) {
        same_press_count = 0;
    } else {
        same_press_count++;
    }

    bool should_send = changed || (same_press_count >= FORCE_SEND_COUNT);

    if (!should_send) {
        ESP_LOGI(TAG, "Press unchanged (%.1f hPa), skip [%d/%d]",
                 data->pressure, same_press_count, FORCE_SEND_COUNT);
        return;
    }

    if (same_press_count >= FORCE_SEND_COUNT) {
        same_press_count = 0;
    }

    last_press = data->pressure;

    ESP_LOGI(TAG, "Press: %.1f hPa [%s]",
             data->pressure, changed ? "changed" : "force");

    mqtt_publish_float("sensors/pressure", data->pressure, "pressure");
}