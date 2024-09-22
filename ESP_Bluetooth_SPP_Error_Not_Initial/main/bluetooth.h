#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spp_api.h" 
void startBluetooth(void);
void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

#endif 