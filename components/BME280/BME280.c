#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "unity.h"
#include "esp_log.h"
#include "bme280.h"
#include "i2c_bus.h"

#include "BME280.h"

#include "driver/spi_master.h"

#define I2C_MASTER_SCL_IO           GPIO_NUM_2           /*!< gpio number for I2C master clock IO2*/
#define I2C_MASTER_SDA_IO           GPIO_NUM_1           /*!< gpio number for I2C master data  IO1*/
#define I2C_MASTER_NUM              I2C_NUM_0            /*!< I2C port number for master bme280 */
#define I2C_MASTER_TX_BUF_DISABLE   0                    /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                    /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ          100000               /*!< I2C master clock frequency */


i2c_bus_handle_t i2c_bus = NULL;
bme280_handle_t bme280 = NULL;


void bme280_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_bus = i2c_bus_create(I2C_MASTER_NUM, &conf);
    bme280 = bme280_create(i2c_bus, BME280_I2C_ADDRESS_DEFAULT);
    ESP_LOGI("BME280:", "bme280_default_init:%d", bme280_default_init(bme280));

}

// void bme280_getdata(void) {
//     int cnt = 10;
//     while (cnt--) {
//         float temperature = 0.0, humidity = 0.0, pressure = 0.0;
//         if (ESP_OK == bme280_read_temperature(bme280, &temperature)) {
//             ESP_LOGI("BME280", "temperature:%f ", temperature);
//         }
//         vTaskDelay(300 / portTICK_RATE_MS);
//         if (ESP_OK == bme280_read_humidity(bme280, &humidity)) {
//             ESP_LOGI("BME280", "humidity:%f ", humidity);
//         }
//         vTaskDelay(300 / portTICK_RATE_MS);
//         if (ESP_OK == bme280_read_pressure(bme280, &pressure)) {
//             ESP_LOGI("BME280", "pressure:%f\n", pressure);
//         }
//         vTaskDelay(300 / portTICK_RATE_MS);
//     }
// } from espressif docs