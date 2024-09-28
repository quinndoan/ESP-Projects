#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"
#include "nvs_flash.h"
#include <string.h>
#include "esp_mac.h"
#include "esp_pm.h"

static const char *TAG = "BLUETOOTH_SPP";
static const char *MESSAGE = "Hello World";
static uint32_t spp_handle = 0;

void spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch (event)
    {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(TAG, "SPP initialized");
        esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, "SPP_SERVER");
        break;

    case ESP_SPP_DATA_IND_EVT:
    {
        char *data = (char *)malloc(param->data_ind.len + 1);
        if (data)
        {
            memcpy(data, param->data_ind.data, param->data_ind.len);
            data[param->data_ind.len] = '\0';
            ESP_LOGI(TAG, "Received data: %s", data);
            free(data);

            // Gửi phản hồi đến điện thoại
            esp_spp_write(spp_handle, strlen(MESSAGE), (uint8_t *)MESSAGE);
            ESP_LOGI(TAG, "Sent response: %s", MESSAGE);
        }
        break;
    }

    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI(TAG, "SPP connection opened");
        spp_handle = param->srv_open.handle;
        break;

    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI(TAG, "SPP connection closed");
        spp_handle = 0;  // Đặt lại spp_handle khi kết nối đóng
        break;

    default:
        break;
    }
}

void send_message_task(void *param)
{
    while (true)
    {
        if (spp_handle != 0) // Kiểm tra xem kết nối đã được thiết lập chưa
        {
            const char *message = "Hello from ESP32!";
            esp_spp_write(spp_handle, strlen(message), (uint8_t *)message);
            ESP_LOGI(TAG, "Sent message: %s", message);
        }
        vTaskDelay(pdMS_TO_TICKS(5000)); // Gửi mỗi 5 giây
    }
}

void startBluetooth(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK)
    {
        ESP_LOGE(TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK)
    {
        ESP_LOGE(TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_init()) != ESP_OK)
    {
        ESP_LOGE(TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_enable()) != ESP_OK)
    {
        ESP_LOGE(TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    esp_bt_gap_set_device_name("myBluetooth");
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

    // Đăng ký callback SPP và khởi tạo SPP
    esp_spp_register_callback(spp_callback);
    esp_spp_init(ESP_SPP_MODE_CB);

    // Đặt mã PIN cố định
    esp_bt_pin_code_t pin_code = {'1', '2', '3', '4'};
    esp_bt_gap_set_pin(ESP_BT_PIN_TYPE_FIXED, 4, pin_code);

    ESP_LOGI(TAG, "Bluetooth initialized with name: myBluetooth");
}

void app_main(void)
{
    startBluetooth();
    xTaskCreate(send_message_task, "Send Message Task", 4096, NULL, 5, NULL);

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

