// #include <stdio.h>
// #include "freertos/queue.h"
// #include"driver/gpio.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"

// #define CONFIG_LOG_MAXIMUM_LEVEL 5
// void handle_button(void*p){
//     gpio_config((gpio_config_t*)p);
//     bool lastState = 1;         // dien tro dang o muc High

//     while(1){
//         bool curState = gpio_get_level(GPIO_NUM_23);        // here is get level, not set, vi bam thi can xem input la gi thoi
//         if (lastState ==1 && curState ==0){
//             ESP_LOGW("Button", "Pressed");
//         }
//         lastState = curState;
//         vTaskDelay(50);
       
//     }
// }

// void setup_button(){
//     gpio_config_t* io_config = (gpio_config_t*)(malloc(sizeof(gpio_config_t*)));
    
//     io_config->pin_bit_mask = (1<< GPIO_NUM_23 );      //     Here add button and setup with GPIO23
//     io_config->mode = GPIO_MODE_INPUT;              // Here button is input
//     io_config->pull_up_en = GPIO_PULLUP_DISABLE;
//     io_config->pull_down_en = GPIO_PULLDOWN_DISABLE;
//     io_config->intr_type = GPIO_INTR_DISABLE;
    
//     xTaskCreate(
//             handle_button,
//         "handle_button",
//         2048,
//         (void*)io_config,
//         1, NULL
//     );
// }

// void handle_leds(void *p){
//     gpio_config((gpio_config_t*)p);
//     while(1){
//         gpio_set_level(GPIO_NUM_2,1);
//         vTaskDelay(50);
//         gpio_set_level(GPIO_NUM_2,0);
//         vTaskDelay(50);
//     }
// };

// void setup_leds(){
//     gpio_config_t* io_config = (gpio_config_t*)(malloc(sizeof(gpio_config_t*)));
    
//     io_config->pin_bit_mask = (1<< GPIO_NUM_2);
//     io_config->mode = GPIO_MODE_OUTPUT;     
//     io_config->pull_up_en = GPIO_PULLUP_DISABLE;
//     io_config->pull_down_en = GPIO_PULLDOWN_DISABLE;
//     io_config->intr_type = GPIO_INTR_DISABLE;
    
//     xTaskCreate(
//             handle_leds,
//         "handle_leds",
//         2048,
//         (void*)io_config,
//         1, NULL
//     );
// }

// void app_main(){
//     setup_leds();
//     setup_button();
// }

#include <stdio.h>
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define CONFIG_LOG_MAXIMUM_LEVEL 5

void handle_button(void *p){
    gpio_config((gpio_config_t*)p);
    bool lastState = 1;  // Điện trở đang ở mức High

    while(1){
        bool curState = gpio_get_level(GPIO_NUM_23);  // Lấy trạng thái hiện tại của nút bấm
        if (lastState == 1 && curState == 0){
            ESP_LOGW("Button", "Pressed");
        }
        lastState = curState;
        vTaskDelay(pdMS_TO_TICKS(50));  // Thời gian delay chính xác
    }
}

void setup_button(){
    gpio_config_t* io_config = (gpio_config_t*)malloc(sizeof(gpio_config_t));
    
    io_config->pin_bit_mask = (1 << GPIO_NUM_23);
    io_config->mode = GPIO_MODE_INPUT;
    io_config->pull_up_en = GPIO_PULLUP_DISABLE;
    io_config->pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config->intr_type = GPIO_INTR_DISABLE;
    
    xTaskCreate(
        handle_button,
        "handle_button",
        2048,
        (void*)io_config,
        1, NULL
    );
}

void handle_leds(void *p){
    gpio_config((gpio_config_t*)p);
    while(1){
        gpio_set_level(GPIO_NUM_2, 1);
        vTaskDelay(pdMS_TO_TICKS(50));
        gpio_set_level(GPIO_NUM_2, 0);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void setup_leds(){
    gpio_config_t* io_config = (gpio_config_t*)malloc(sizeof(gpio_config_t));
    
    io_config->pin_bit_mask = (1 << GPIO_NUM_2);
    io_config->mode = GPIO_MODE_OUTPUT;     
    io_config->pull_up_en = GPIO_PULLUP_DISABLE;
    io_config->pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config->intr_type = GPIO_INTR_DISABLE;
    
    xTaskCreate(
        handle_leds,
        "handle_leds",
        2048,
        (void*)io_config,
        1, NULL
    );
}

void app_main(){
    setup_leds();
    setup_button();
}
