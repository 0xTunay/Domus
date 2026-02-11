#ifndef MOSQUITTO_H
#define MOSQUITTO_H


static void mqtt_app_start(void);
static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data);
#endif // MOSQUITTO_H