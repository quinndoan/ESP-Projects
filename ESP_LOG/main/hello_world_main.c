/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "esp_log.h"
#include"esp_system.h"
static const char *TAG = "main";
void myLogger(const char *str, va_list arg){
    printf("Log cua Quinn\n");
    vprintf(str,arg);
}
void app_main(void)
{
    esp_log_set_vprintf(myLogger);
    ESP_LOGE(TAG, "ESP_LOGE");
    ESP_LOGW(TAG, "ESP_LOGW");
    ESP_LOGI(TAG, "ESP_LOGI");
    ESP_LOGD(TAG, "ESP_LOGD");
    ESP_LOGV(TAG, "ESP_LOGV");

}
