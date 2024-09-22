#include <stdio.h>
#include "bluetooth.h"
#include"string.h"
#include "esp_bt.h"  // Thư viện cho Bluetooth
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_hidd_api.h"
#include "esp_spp_api.h"
#include "nvs_flash.h"
#include "esp_log.h"

// Buffer để gửi nhận dữ liệu
MessageBufferHandle_t typeMessage = NULL;

void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
        case ESP_SPP_INIT_EVT: {
            esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, "SPP_SERVER");
            break;
        }

        case ESP_SPP_DATA_IND_EVT: {
            char *data = (char*) malloc(param->data_ind.len + 1);
            if (data != NULL) {
                memset(data, 0, param->data_ind.len + 1);
                memcpy(data, param->data_ind.data, param->data_ind.len);
                ESP_LOGI("BLT", "Received data: %s", data);
                free(data);
            }
            break;
        }

        default:
            break;
    }
}

// Hàm khởi động Bluetooth
void startBluetooth(void){
    // Khởi tạo NVS (Non-Volatile Storage)
    if (nvs_flash_init() != ESP_OK){
        nvs_flash_erase();  // Xóa dữ liệu cũ nếu NVS đã được khởi tạo trước đó
        nvs_flash_init();    // Khởi tạo lại NVS
    }

    // Cấu hình bộ điều khiển Bluetooth
    esp_bt_controller_config_t *bt_cfg = (esp_bt_controller_config_t *) malloc(sizeof(esp_bt_controller_config_t));
    bt_cfg-> mode = ESP_BT_MODE_CLASSIC_BT;
    esp_bt_controller_init(bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);

    // Khởi tạo và bật Bluedroid (ngăn xếp giao thức của ESP32 cho Bluetooth)
    esp_bluedroid_config_t *bluedroid_cfg = (esp_bluedroid_config_t *) malloc(sizeof(esp_bluedroid_config_t));
    bluedroid_cfg->ssp_en = false; 
    esp_bluedroid_init_with_cfg(bluedroid_cfg);
    esp_bluedroid_enable();

    // Đặt tên cho thiết bị Bluetooth
    esp_bt_gap_set_device_name("quinnBT");

    // Đặt chế độ quét (kết nối và khả năng khám phá thiết bị)
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

    esp_spp_register_callback(esp_spp_cb);
    // Khởi tạo SPP với cấu hình chế độ callback
    esp_spp_enhanced_init(ESP_SPP_MODE_CB);

    // Đặt mã PIN cho thiết bị
    esp_bt_pin_code_t pin_code = {'1', '2', '3', '4'};
    esp_bt_gap_set_pin(ESP_BT_PIN_TYPE_FIXED, 4, pin_code);

    free(bt_cfg);
    free(bluedroid_cfg);
}

