// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/event_groups.h"
// #include "esp_wifi.h"
// #include "esp_event.h"
// #include "esp_log.h"
// #include "nvs_flash.h"
// #include<string.h>

// // Khai báo SSID và Password cho AP mode và STA mode
// #define AP_SSID "ESP32-AP"
// #define AP_PASSWORD "12345678"
// #define STA_SSID "Quinn"
// #define STA_PASSWORD "20102010"

// // Tag cho log
// static const char *TAG = "WiFi_Mode_Switch";

// // Biến Event Group để quản lý trạng thái Wi-Fi
// static EventGroupHandle_t wifi_event_group;
// const int CONNECTED_BIT = BIT0;

// // Biến điều khiển mode hiện tại
// bool isAPMode = true;

// // Event Handler cho Wi-Fi
// static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
//     if (event_base == WIFI_EVENT) {
//         switch (event_id) {
//             case WIFI_EVENT_AP_START:
//                 ESP_LOGI(TAG, "AP Mode: Started");
//                 break;
//             case WIFI_EVENT_STA_START:
//                 ESP_LOGI(TAG, "STA Mode: Started");
//                 esp_wifi_connect();
//                 break;
//             case WIFI_EVENT_STA_DISCONNECTED:
//                 ESP_LOGI(TAG, "STA Mode: Disconnected, retrying...");
//                 esp_wifi_connect();
//                 xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
//                 break;
//         }
//     } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
//         ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
//         ESP_LOGI(TAG, "STA Mode: Connected, IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
//         xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
//     }
// }

// // Hàm chuyển đổi giữa AP và STA mode
// void switch_mode() {
//     esp_wifi_disconnect(); // Ngắt kết nối hiện tại

//     if (isAPMode) {
//         // Cấu hình AP mode
//         ESP_ERROR_CHECK(esp_wifi_stop());
//         wifi_config_t wifi_config = {
//             .ap = {
//                 .ssid = AP_SSID,
//                 .password = AP_PASSWORD,
//                 .ssid_len = strlen(AP_SSID),
//                 .max_connection = 4,
//                 .authmode = WIFI_AUTH_WPA_WPA2_PSK
//             }
//         };
//         ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
//         ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
//         ESP_ERROR_CHECK(esp_wifi_start());
//         ESP_LOGI(TAG, "Switched to AP Mode");
//     } else {
//         // Cấu hình STA mode
//         ESP_ERROR_CHECK(esp_wifi_stop());
//         wifi_config_t wifi_config = {
//             .sta = {
//                 .ssid = STA_SSID,
//                 .password = STA_PASSWORD
//             }
//         };
//         ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
//         ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
//         ESP_ERROR_CHECK(esp_wifi_start());
//         ESP_LOGI(TAG, "Switched to STA Mode");
//     }
// }

// void app_main(void) {
//     // Khởi tạo NVS
//     ESP_ERROR_CHECK(nvs_flash_init());
//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());
//     esp_netif_create_default_wifi_sta();
//     esp_netif_create_default_wifi_ap();

//     wifi_event_group = xEventGroupCreate();

//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     ESP_ERROR_CHECK(esp_wifi_init(&cfg));
//     ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
//     ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

//     // Khởi tạo Wi-Fi ở chế độ đầu tiên (AP mode)
//     switch_mode();

//     // Thay đổi giữa AP và STA mode mỗi 10 giây
//     while (true) {
//         vTaskDelay(10000 / portTICK_PERIOD_MS); // Đợi 10 giây
//         isAPMode = !isAPMode;
//         switch_mode();
//     }
// }

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
#include "esp_http_server.h"

#define AP_SSID "ESP32-AP"
#define AP_PASSWORD "12345678"
#define MAX_STA_CONN 5

static const char *TAG = "WiFi_Web_Config";
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
const int CONNECT_FAIL_BIT = BIT1;

char ssid[32] = {0};
char password[64] = {0};

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
                ESP_LOGI(TAG, "STA Mode: Disconnected");
                xEventGroupSetBits(wifi_event_group, CONNECT_FAIL_BIT);
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "STA Mode: Connected, IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    }
}

static esp_err_t wifi_sta_connect(const char *ssid, const char *password) {
    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    return ESP_OK;
}

static void wifi_ap_start() {
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .password = AP_PASSWORD,
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
    ESP_LOGI(TAG, "AP Mode: SSID: %s, Password: %s", AP_SSID, AP_PASSWORD);
}

static esp_err_t index_get_handler(httpd_req_t *req) {
    char response[] = "<!DOCTYPE html><html><body>"
                      "<h1>WiFi Configuration</h1>"
                      "<form action=\"/submit\" method=\"post\">"
                      "SSID: <input type=\"text\" name=\"ssid\"><br>"
                      "Password: <input type=\"password\" name=\"password\"><br>"
                      "<input type=\"submit\" value=\"Submit\">"
                      "</form></body></html>";
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

static esp_err_t submit_post_handler(httpd_req_t *req) {
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)));
        if (ret <= 0) {
            return ESP_FAIL;
        }
        remaining -= ret;
    }
    buf[req->content_len] = '\0';

    sscanf(buf, "ssid=%31[^&]&password=%63[^\n]", ssid, password);

    ESP_LOGI(TAG, "Received SSID: %s, Password: %s", ssid, password);

    // Lưu thông tin vào NVS hoặc sử dụng trực tiếp ssid, password cho hàm wifi_sta_connect
    wifi_sta_connect(ssid, password);

    char response[] = "<html><body><h1>Connecting to WiFi...</h1></body></html>";
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

static httpd_uri_t uri_get = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = index_get_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_post = {
    .uri      = "/submit",
    .method   = HTTP_POST,
    .handler  = submit_post_handler,
    .user_ctx = NULL
};

static httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
    }
    return server;
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
    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_event_group = xEventGroupCreate();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_ap_start();
    start_webserver();

    while (1) {
        EventBits_t bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | CONNECT_FAIL_BIT, pdTRUE, pdFALSE, portMAX_DELAY);

        if (bits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "Connected to WiFi in STA mode.");
            vTaskDelay(portMAX_DELAY);
        } else if (bits & CONNECT_FAIL_BIT) {
            ESP_LOGI(TAG, "Failed to connect, restarting AP mode.");
            wifi_ap_start();
            start_webserver();
        }
    }
}
