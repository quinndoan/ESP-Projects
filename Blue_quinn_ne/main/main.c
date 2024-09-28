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
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SPP_SERVER_NAME "SPP_SERVER"
#define EXAMPLE_DEVICE_NAME "ESP32_SPP"
#define SPP_DATA_MAX_LEN 512

static const char *TAG = "SPP_RECEIVER";

static char ssid[32] = {0};
static char password[64] = {0};
static int wifi_mode = 1; // Default to AP mode

// SPP callback function to handle SPP events
static void spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(TAG, "ESP_SPP_INIT_EVT");
        esp_bt_dev_set_device_name(EXAMPLE_DEVICE_NAME);
        esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
        esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, SPP_SERVER_NAME);
        break;

    case ESP_SPP_DATA_IND_EVT: {
        ESP_LOGI(TAG, "ESP_SPP_DATA_IND_EVT len=%d handle=%d", param->data_ind.len, param->data_ind.handle);
        
        if (param->data_ind.len < SPP_DATA_MAX_LEN) {
            char incoming_data[SPP_DATA_MAX_LEN] = {0};
            strncpy(incoming_data, (char *)param->data_ind.data, param->data_ind.len);

            ESP_LOGI(TAG, "Received data: %s", incoming_data);

            // Parse incoming data for SSID and password
            sscanf(incoming_data, "ssid=%[^&]&password=%s", ssid, password);

            ESP_LOGI(TAG, "Parsed SSID: %s, Password: %s", ssid, password);

            // Save SSID and Password to NVS
            nvs_handle_t nvs_handle;
            esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
            if (err == ESP_OK) {
                nvs_set_str(nvs_handle, "ssid", ssid);
                nvs_set_str(nvs_handle, "password", password);
                nvs_set_i32(nvs_handle, "wifi_mode", 0); // Set mode to STA mode
                nvs_commit(nvs_handle);
                nvs_close(nvs_handle);

                // Inform user of successful receipt
                const char response[] = "SSID and Password received. Switching to STA mode.";
                esp_spp_write(param->data_ind.handle, strlen(response), (uint8_t *)response);

                // Restart ESP32 to apply new Wi-Fi settings
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                esp_restart();
            } else {
                ESP_LOGE(TAG, "Failed to open NVS. Error: %s", esp_err_to_name(err));
            }
        }
        break;
    }

    case ESP_SPP_CONG_EVT:
        ESP_LOGI(TAG, "ESP_SPP_CONG_EVT");
        break;

    case ESP_SPP_WRITE_EVT:
        ESP_LOGI(TAG, "ESP_SPP_WRITE_EVT len=%d cong=%d", param->write.len, param->write.cong);
        break;

    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI(TAG, "ESP_SPP_SRV_OPEN_EVT, handle = %d", param->srv_open.handle);
        break;

    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI(TAG, "ESP_SPP_CLOSE_EVT, handle = %d", param->close.handle);
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

    // Register SPP callback and initialize SPP
    esp_spp_register_callback(spp_callback);
    esp_spp_init(ESP_SPP_MODE_CB);
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

    ESP_LOGI(TAG, "Bluetooth SPP receiver is up and running.");
}
