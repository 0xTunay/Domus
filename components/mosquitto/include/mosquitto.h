#ifndef MOSQUITTO_H
#define MOSQUITTO_H
#include "esp_event_base.h"
#include "mqtt_client.h"
void mqtt_app_start(void);
extern esp_mqtt_client_handle_t s_client;
extern SemaphoreHandle_t s_mqtt_mutex;

#endif // MOSQUITTO_H