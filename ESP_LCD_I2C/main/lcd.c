#include"freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "lcd.h"
#include "esp_log.h"

#include "hd44780.h"
void handle_lcd(void*p){
    int count =0;
    while(1){

    }
}
void setup_led(){
    hd44780_t *lcd = (hd44780_t*)malloc(sizeof (hd44780_t*));
    lcd->write_cb = NULL;
    lcd->font = HD44780_FONT_5X8;
    lcd->lines =2;
    lcd->backlight = true;
    lcd->pins.rs =2;
    
    xTaskCreate(handle_lcd,"handle_lcd", 2048,(void*)io_config,1,NULL);

}

