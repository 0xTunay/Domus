#include "Sensors_types.h"

#ifndef RECEIVE_QUEUE_H
#define RECEIVE_QUEUE_H

#define ITEM_SIZE 50

void receive_queue_temperature(SensorData_t *data);
void receive_queue_humidity(SensorData_t *data);
void receive_queue_pressure(SensorData_t *data);


#endif // RECEIVE_QUEUE_H