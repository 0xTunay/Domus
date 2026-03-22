#ifndef BME280_H
#define BME280_H


#include "i2c_bus.h"
#include "bme280.h"

extern i2c_bus_handle_t i2c_bus;
extern bme280_handle_t bme280;


void bme280_init(void);
// void bme280_getdata(void);
void i2c_bus_init(void);
#endif // BME280_H