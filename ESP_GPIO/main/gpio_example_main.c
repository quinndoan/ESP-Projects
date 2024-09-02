#include<stdio.h>
#include"driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
void app_main(void){
    gpio_config_t io_config ={};
    io_config.pin_bit_mask = (1<< GPIO_NUM_2);
    io_config.mode = GPIO_MODE_OUTPUT;
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_config);

    while(1){
        gpio_set_level(GPIO_NUM_2,1);
        vTaskDelay(50);
        gpio_set_level(GPIO_NUM_2,0);
        vTaskDelay(50);
    }
}
