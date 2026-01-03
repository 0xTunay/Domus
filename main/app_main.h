//
// Created by tunay on 12/26/25.
//

#ifndef _APP_MAIN_H
#define _APP_MAIN_H
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_GPIO GPIO_NUM_2
#define DHT_GPIO GPIO_NUM_19

#define ITEM_SIZE 50

static bool BlinkEnable = false;

void LedInit(void) {
	gpio_config_t io_conf = {};
	io_conf.intr_type = GPIO_INTR_DISABLE;          // без прерываний
	io_conf.mode = GPIO_MODE_OUTPUT;                // выход
	io_conf.pin_bit_mask = 1ULL << LED_GPIO;        // маска пина
	io_conf.pull_down_en = 0;                       // без подтяжки вниз
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);
}
void LedON(void)  { gpio_set_level(LED_GPIO, 1); }
void LedOFF(void) { gpio_set_level(LED_GPIO, 0); }
void LedBlink(void *pvParameter) {
	while (1) {
		if (BlinkEnable) {
			gpio_set_level(LED_GPIO, 1);
			vTaskDelay(pdMS_TO_TICKS(350));
			gpio_set_level(LED_GPIO, 0);
			vTaskDelay(pdMS_TO_TICKS(350));
		}
		else {
			vTaskDelay(pdMS_TO_TICKS(50));
		}
	}
}
void LedStopBlink(void) {
	gpio_set_level(LED_GPIO, 0);
}
#endif //_APP_MAIN_H