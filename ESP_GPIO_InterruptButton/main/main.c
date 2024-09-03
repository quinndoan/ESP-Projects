#include <stdio.h>
#include"driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
void handle_leds(void *p){
    gpio_config((gpio_config_t*)p);
    while(1){
        gpio_set_level(GPIO_NUM_2,1);
        vTaskDelay(50);
        gpio_set_level(GPIO_NUM_2,0);
        vTaskDelay(50);
    }
}

void setup_leds(){
    gpio_config_t* io_config = (gpio_config_t*)(malloc(sizeof(gpio_config_t*)));
    
    io_config->pin_bit_mask = (1<< GPIO_NUM_2);
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
}