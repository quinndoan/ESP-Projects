#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"

#define DEVICE_NAME "ESP32_BT_SPP"

static const char *TAG = "ESP32_BLUETOOTH_SPP";

// Callback function khi nhận được dữ liệu từ điện thoại
void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(TAG, "ESP SPP INIT");
        esp_bt_dev_set_device_name(DEVICE_NAME);
        esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, "ESP32_SPP_SERVER");
        break;
    case ESP_SPP_START_EVT:
        ESP_LOGI(TAG, "SPP server started");
        break;
    case ESP_SPP_DATA_IND_EVT:
        ESP_LOGI(TAG, "Received data");
        // Hiển thị dữ liệu nhận được từ điện thoại
        ESP_LOGI(TAG, "Received data: %.*s", param->data_ind.len, (char *)param->data_ind.data);
        break;
    default:
        break;
    }
}

void app_main(void) {
    // Khởi tạo Bluetooth
    esp_err_t ret = esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "Bluetooth controller release failed");
        return;
    }

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "Bluetooth controller initialize failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (ret) {
        ESP_LOGE(TAG, "Bluetooth controller enable failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "Bluedroid initialize failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "Bluedroid enable failed: %s", esp_err_to_name(ret));
        return;
    }

    // Đăng ký callback cho SPP
    ret = esp_spp_register_callback(esp_spp_cb);
    if (ret) {
        ESP_LOGE(TAG, "SPP register callback failed: %s", esp_err_to_name(ret));
        return;
    }

    // Khởi tạo SPP
    ret = esp_spp_init(ESP_SPP_MODE_CB);
    if (ret) {
        ESP_LOGE(TAG, "SPP init failed: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "Bluetooth SPP initialized and ready.");
}
