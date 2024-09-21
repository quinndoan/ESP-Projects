#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_netif.h"

#define AP_SSID "ESP32-AP"
#define AP_PASSWORD "12345678"
#define MAX_STA_CONN 5
#define MIN(a, b) ((a) < (b) ? (a) : (b))


static const char *TAG = "WiFi_Switch_Mode";
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

char ssid[32] = "default_ssid";
char password[64] = "default_password";

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_AP_START:
                ESP_LOGI(TAG, "AP Mode: Started");
                break;
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "STA Mode: Started");
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "STA Mode: Disconnected, retrying...");
                esp_wifi_connect();
                xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "STA Mode: Connected, IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    }
}

void switch_to_sta_mode(const char *new_ssid, const char *new_password) {
    ESP_LOGI(TAG, "Switching to STA Mode...");
    // Ngắt kết nối hiện tại
    esp_wifi_disconnect();
    esp_wifi_stop();

    // Cập nhật SSID và Password mới
    strncpy(ssid, new_ssid, sizeof(ssid));
    strncpy(password, new_password, sizeof(password));

    // Cấu hình lại chế độ STA
    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
}

void switch_to_ap_mode() {
    ESP_LOGI(TAG, "Switching to AP Mode...");
    esp_wifi_disconnect();
    esp_wifi_stop();

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_SSID,
            .password = AP_PASSWORD,
            .ssid_len = strlen(AP_SSID),
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        }
    };

    if (strlen(AP_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_event_group = xEventGroupCreate();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    // Bắt đầu với AP mode
    switch_to_ap_mode();

    // Giả lập: sau 15 giây, chuyển sang STA mode với SSID và Password mới
    vTaskDelay(15000 / portTICK_PERIOD_MS);

    // Thử kết nối với mạng Wi-Fi mới (SSID và Password bạn cần nhập)
    switch_to_sta_mode("your_new_ssid", "your_new_password");

    // Giả lập: sau 15 giây, chuyển về AP mode nếu cần
    // vTaskDelay(15000 / portTICK_PERIOD_MS);
    // switch_to_ap_mode();
}
