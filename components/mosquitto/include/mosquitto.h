#ifndef MOSQUITTO_H
#define MOSQUITTO_H
#include "esp_event_base.h"

void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data);
void mqtt_app_start(void);
void log_error_if_nonzero(const char *msg, int error_code);
extern esp_mqtt_client_handle_t s_client;
extern SemaphoreHandle_t MqttSemaphoreHandle;

#endif // MOSQUITTO_H