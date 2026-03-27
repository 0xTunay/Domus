#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <inttypes.h>
#include "freertos/semphr.h"
#include "nvs_flash.h"

#include "receive_queue.h"
#include "mosquitto.h"
#include "espnow_handler.h"
#include "Sensors_types.h"

static const char *TAG = "[Receive Queue]";

QueueHandle_t SensorQueueHandle = NULL;
QueueHandle_t HumidityQueueHandle = NULL;
QueueHandle_t PressureQueueHandle = NULL;


void receive_queue_temperature(SensorData_t *data) {

  const TickType_t xTicksToWait = pdMS_TO_TICKS(5000);
  float last_temp = UINT32_MAX;

  int same_temp_count = 0;

  const int FORCE_SEND_COUNT = 5;
  char payload[64];
  if (xQueueReceive(SensorQueueHandle, &data->temperature, xTicksToWait) == pdTRUE) {
    bool changed = (data->temperature != last_temp);

    if (changed) {
      same_temp_count = 0;
    } else {
      same_temp_count++;
    }

    bool should_send = changed || (same_temp_count >= FORCE_SEND_COUNT);

    if (should_send) {
      if (same_temp_count >= FORCE_SEND_COUNT) {
        same_temp_count = 0;
      }

      last_temp = data->temperature;
      float temperature = data->temperature / 10.0f;
      snprintf(payload, sizeof(payload), "%.1f", temperature);

      ESP_LOGI(TAG, "Received temp: %" PRIu32 " (%.1f C) [%s]",
          data->temperature, temperature, changed ? "changed" : "force");

      if (s_client != NULL && xSemaphoreTake(s_mqtt_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        int msg_id = esp_mqtt_client_publish(s_client, "sensors/temperature", payload, 0, 1, 0);
        ESP_LOGI(TAG, "Published temp: %s, msg_id=%d", payload, msg_id);
        xSemaphoreGive(s_mqtt_mutex);
      } else{
        ESP_LOGW(TAG, "MQTT not ready or mutex timeout");
      }
    } else {
      ESP_LOGI(TAG, "Temp unchanged (%.1f C), skip [%d/%d]",
          data->temperature / 10.0f, same_temp_count, FORCE_SEND_COUNT);
    }
  }
  SensorQueueHandle = xQueueCreate(ITEM_SIZE, sizeof(uint32_t));
  if (SensorQueueHandle == NULL) {
    ESP_LOGE(TAG, "Failed to create SensorTaskQueue");
    return;
  }
}
void receive_queue_humidity(SensorData_t *data) {
  const TickType_t xTicksToWait = pdMS_TO_TICKS(5000);
  float last_hum = UINT32_MAX;

  int same_hum_count = 0;

  const int FORCE_SEND_COUNT = 5;
  char payload[64];
  if (xQueueReceive(HumidityQueueHandle, &data->humidity, xTicksToWait) == pdTRUE) {
    bool changed = (data->humidity != last_hum);

    if (changed) {
      same_hum_count = 0;
    } else {
      same_hum_count++;
    }

    bool should_send = changed || (same_hum_count >= FORCE_SEND_COUNT);

    if (should_send) {
      if (same_hum_count >= FORCE_SEND_COUNT) {
        same_hum_count = 0;
      }

      last_hum = data->humidity;
      float humidity = data->humidity / 10.0f;
      snprintf(payload, sizeof(payload), "%.1f", humidity);

      ESP_LOGI(TAG, "Received hum: %" PRIu32 " (%.1f %%) [%s]",
               data->humidity, humidity, changed ? "changed" : "force");

      if (s_client != NULL && xSemaphoreTake(s_mqtt_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        int msg_id = esp_mqtt_client_publish(s_client, "sensors/humidity", payload, 0, 1, 0);
        ESP_LOGI(TAG, "Published hum: %s, msg_id=%d", payload, msg_id);
        xSemaphoreGive(s_mqtt_mutex);
      } else {
        ESP_LOGW(TAG, "MQTT not ready or mutex timeout");
      }
    } else {
      ESP_LOGI(TAG, "Hum unchanged (%.1f %%), skip [%d/%d]",
               data->humidity / 10.0f, same_hum_count, FORCE_SEND_COUNT);
    }
  }
  HumidityQueueHandle = xQueueCreate(ITEM_SIZE, sizeof(uint32_t));
  if (HumidityQueueHandle == NULL) {
    ESP_LOGE(TAG, "Failed to create SensorTaskQueue");
    return;
  }
}
void receive_queue_pressure(SensorData_t *data) {
  const TickType_t xTicksToWait = pdMS_TO_TICKS(5000);
  float last_press = UINT32_MAX;

  int same_press_count = 0;

  const int FORCE_SEND_COUNT = 5;
  char payload[64];

  if (xQueueReceive(HumidityQueueHandle, &data->humidity, xTicksToWait) == pdTRUE) {
    bool changed = (data->humidity != last_press);

    if (changed) {
      same_press_count = 0;
    } else {
      same_press_count++;
    }

    bool should_send = changed || (same_press_count >= FORCE_SEND_COUNT);

    if (should_send) {
      if (same_press_count >= FORCE_SEND_COUNT) {
        same_press_count = 0;
      }

      last_press = data->humidity;
      float humidity = data->humidity / 10.0f;
      snprintf(payload, sizeof(payload), "%.1f", humidity);

      ESP_LOGI(TAG, "Received hum: %" PRIu32 " (%.1f %%) [%s]",
               data->humidity, humidity, changed ? "changed" : "force");

      if (s_client != NULL && xSemaphoreTake(s_mqtt_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        int msg_id = esp_mqtt_client_publish(s_client, "sensors/humidity", payload, 0, 1, 0);
        ESP_LOGI(TAG, "Published hum: %s, msg_id=%d", payload, msg_id);
        xSemaphoreGive(s_mqtt_mutex);
      } else {
        ESP_LOGW(TAG, "MQTT not ready or mutex timeout");
      }
    } else {
      ESP_LOGI(TAG, "Hum unchanged (%.1f %%), skip [%d/%d]",
               data->humidity / 10.0f, same_press_count, FORCE_SEND_COUNT);
    }
  }
  PressureQueueHandle = xQueueCreate(ITEM_SIZE, sizeof(uint32_t));
  if (PressureQueueHandle == NULL) {
    ESP_LOGE(TAG, "Failed to create SensorTaskQueue");
    return;
  }
}
