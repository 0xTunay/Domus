#ifndef BME280_H
#define BME280_H

extern i2c_bus_handle_t i2c_bus;
extern bme280_handle_t bme280;


void bme280_init(void);
// void bme280_getdata(void);

#endif // BME280_H