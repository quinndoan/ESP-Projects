#include <stdio.h>
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// Sử dụng kiểu dữ liệu mới
QueueHandle_t button_queue;
#define CONFIG_LOG_MAXIMUM_LEVEL 5

void handle_button(void *p) {
    gpio_config((gpio_config_t *)p);
    button_queue = xQueueCreate(16, sizeof(bool));
    
    if (button_queue == NULL) {
        ESP_LOGE("Queue", "Failed to create button queue");
        vTaskDelete(NULL);
    }

    bool lastState = 1;  // Điện trở đang ở mức High

    while (1) {
        bool curState = gpio_get_level(GPIO_NUM_23);  // Lấy trạng thái hiện tại của nút bấm
        if (lastState == 1 && curState == 0) {
            ESP_LOGW("Button", "Pressed");
            xQueueSend(button_queue, &curState, portMAX_DELAY);
        }
        lastState = curState;
        vTaskDelay(pdMS_TO_TICKS(50));  // Sử dụng macro chuyển đổi ms sang ticks
    }
}

void setup_button() {
    gpio_config_t io_config = {
        .pin_bit_mask = (1ULL << GPIO_NUM_23),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    xTaskCreate(handle_button, "handle_button", 2048, (void *)&io_config, 1, NULL);
}

void handle_leds(void *p) {
    gpio_config((gpio_config_t *)p);
    int count = 0;

    while (1) {
        bool state;
        if (xQueueReceive(button_queue, &state, portMAX_DELAY)) {
            count++;
            ESP_LOGW("LED", "COUNT: %d", count);
            gpio_set_level(GPIO_NUM_2, 1);
            vTaskDelay(pdMS_TO_TICKS(100));  // Chờ đợi một khoảng thời gian với LED bật
            gpio_set_level(GPIO_NUM_2, 0);
        }
    }
}

void setup_leds() {
    gpio_config_t io_config = {
        .pin_bit_mask = (1ULL << GPIO_NUM_2),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    xTaskCreate(handle_leds, "handle_leds", 2048, (void *)&io_config, 1, NULL);
}

void app_main() {
    setup_leds();
    setup_button();
}
