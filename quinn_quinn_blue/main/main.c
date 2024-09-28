#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SPP_SERVER_NAME "SPP_SERVER"
#define EXAMPLE_DEVICE_NAME "ESP32_SPP"
#define SPP_DATA_MAX_LEN 512

static const char *TAG = "SPP_RECEIVER";

static char ssid[32] = {0};
static char password[64] = {0};

// SPP callback function to handle SPP events
static void spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(TAG, "ESP_SPP_INIT_EVT");
        esp_bt_gap_set_device_name(EXAMPLE_DEVICE_NAME);
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, SPP_SERVER_NAME);
        ESP_LOGI(TAG, "Bluetooth is ready to receive data.");
        break;

    case ESP_SPP_DATA_IND_EVT: {
        ESP_LOGI(TAG, "ESP_SPP_DATA_IND_EVT len=%d handle=%" PRIu32, param->data_ind.len, param->data_ind.handle);

        if (param->data_ind.len < SPP_DATA_MAX_LEN) {
            char incoming_data[SPP_DATA_MAX_LEN] = {0};
            strncpy(incoming_data, (char *)param->data_ind.data, param->data_ind.len);
            incoming_data[param->data_ind.len] = '\0'; // Đảm bảo chuỗi được kết thúc
            ESP_LOGI(TAG, "Received data: %s", incoming_data);

            // Parse incoming data for SSID and password
            if (sscanf(incoming_data, "ssid=%[^&]&password=%s", ssid, password) == 2) {
                ESP_LOGI(TAG, "Parsed SSID: %s, Password: %s", ssid, password);

                // Lưu SSID và mật khẩu vào NVS
                nvs_handle_t nvs_handle;
                esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
                if (err == ESP_OK) {
                    nvs_set_str(nvs_handle, "ssid", ssid);
                    nvs_set_str(nvs_handle, "password", password);
                    nvs_commit(nvs_handle);
                    nvs_close(nvs_handle);

                    // Thông báo cho người dùng đã nhận SSID và mật khẩu thành công
                    const char response[] = "SSID and Password received and saved to NVS.";
                    esp_spp_write(param->data_ind.handle, strlen(response), (uint8_t *)response);

                    ESP_LOGI(TAG, "SSID and Password saved to NVS successfully.");
                } else {
                    ESP_LOGE(TAG, "Failed to open NVS. Error: %s", esp_err_to_name(err));
                }
            } else {
                ESP_LOGE(TAG, "Failed to parse SSID and Password from data: %s", incoming_data);
                const char response[] = "Failed to parse SSID and Password. Please resend.";
                esp_spp_write(param->data_ind.handle, strlen(response), (uint8_t *)response);
            }
        }
        break;
    }

    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI(TAG, "ESP_SPP_SRV_OPEN_EVT, handle = %" PRIu32, param->srv_open.handle);
        break;

    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI(TAG, "ESP_SPP_CLOSE_EVT, handle = %" PRIu32, param->close.handle);
        break;

    default:
        break;
    }
}

void init_bluetooth(void) {
    // Initialize and enable the Bluetooth controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth controller initialization failed");
        return;
    }
    if (esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT) != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth controller enable failed");
        return;
    }

    // Initialize and enable Bluedroid stack
    if (esp_bluedroid_init() != ESP_OK) {
        ESP_LOGE(TAG, "Bluedroid initialization failed");
        return;
    }
    if (esp_bluedroid_enable() != ESP_OK) {
        ESP_LOGE(TAG, "Bluedroid enable failed");
        return;
    }

    // Set device name
    esp_bt_gap_set_device_name(EXAMPLE_DEVICE_NAME);

    // Set discoverable and connectable mode
    esp_err_t status = esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    if (status != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set scan mode, error: %s", esp_err_to_name(status));
    }

    // Register SPP callback and initialize SPP
    esp_spp_register_callback(spp_callback);
    if (esp_spp_init(ESP_SPP_MODE_CB) != ESP_OK) {
        ESP_LOGE(TAG, "SPP initialization failed");
        return;
    }

    // Start the SPP service
    esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, SPP_SERVER_NAME);

    ESP_LOGI(TAG, "Bluetooth SPP initialized and ready to be discovered.");
}

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Bluetooth
    init_bluetooth();

    ESP_LOGI(TAG, "Bluetooth SPP receiver is up and running. Please send SSID and Password from your phone.");
}
