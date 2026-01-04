#ifndef DISPLAY_LVGL_H
#define DISPLAY_LVGL_H
#include "esp_err.h"

esp_err_t DisplayLvglInit(void);
esp_err_t DisplayLvglUpdateTemp(float temperature);
void DisplayLvglTask(void *pvParameters);

#endif // __DISPLAY_LVGL_H__
