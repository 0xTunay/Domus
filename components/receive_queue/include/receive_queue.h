#include "Sensors_types.h"

#ifndef RECEIVE_QUEUE_H
#define RECEIVE_QUEUE_H

#define QUEUE_SIZE 10

void receive_queue_init(void);

void receive_queue_temperature(SensorData_t *data);
void receive_queue_humidity(SensorData_t *data);
void receive_queue_pressure(SensorData_t *data);

extern QueueHandle_t TemperatureQueueHandle;
extern QueueHandle_t HumidityQueueHandle;
extern QueueHandle_t PressureQueueHandle;
#endif // RECEIVE_QUEUE_H