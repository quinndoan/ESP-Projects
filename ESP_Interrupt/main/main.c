#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "esp_log.h"

QueueHandle_t button_queue;
#define CONFIG_LOG_MAXIMUM_LEVEL 5

void handle_button(void *p){
    gpio_config((gpio_config_t*)p);
    button_queue = xQueueCreate(16,sizeof(bool));
    vTaskDelete(NULL);
    bool lastState = 1;  // Điện trở đang ở mức High

    while(1){
        bool curState = gpio_get_level(GPIO_NUM_23);  // Lấy trạng thái hiện tại của nút bấm
        if (lastState == 1 && curState == 0){
            ESP_LOGW("Button", "Pressed");
            xQueueSend(button_queue,&curState ,portMAX_DELAY);
        }
        lastState = curState;
        vTaskDelay(50);  // Thời gian delay chính xác
    }
}
void gpio_isr_handle(void *p){
    gpio_intr_disable(GPIO_NUM_23);
    ESP_DRAM_LOGW("gpio_isr_handle","Entered interrupt");
    bool curState =1;
    xQueueSendFromISR(button_queue, &curState,pdFALSE);
    gpio_intr_enable(GPIO_NUM_23);
}

void setup_button(){
    gpio_config_t* io_config = (gpio_config_t*)malloc(sizeof(gpio_config_t));
    
    io_config->pin_bit_mask = (1 << GPIO_NUM_23);
    io_config->mode = GPIO_MODE_INPUT;
    io_config->pull_up_en = GPIO_PULLUP_DISABLE;
    io_config->pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config->intr_type = GPIO_INTR_NEGEDGE;
    
     gpio_install_isr_service(0);
     gpio_isr_handler_add(GPIO_NUM_23,gpio_isr_handle,NULL);
   //gpio_isr_register(gpio_isr_handle,NULL,0,NULL);      // hàm phục vụ ngắt
}

void handle_leds(void *p){
    gpio_config((gpio_config_t*)p);
    int count=0;
    while(1){
        bool state;
        if (xQueueReceive(button_queue, &state, portMAX_DELAY)){
            count++;
            ESP_LOGW("LED", "COUNT: %d", count);
        }
        gpio_set_level(GPIO_NUM_2, 1);
        vTaskDelay(50);
        gpio_set_level(GPIO_NUM_2, 0);
        vTaskDelay(50);
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
