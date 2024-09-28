extern "C" {#include <stdio.h>
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "algorithm"
void startBluetooth(void)
{
  if (nvs_flash_init() != ESP_OK)
  {
    nvs_flash_erase();
    nvs_flash_init();
  }
  esp_bt_controller_config_t* cfg = new esp_bt_controller_config_t BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  cfg->mode = ESP_BT_MODE_CLASSIC_BT;
  esp_bt_controller_init(cfg);
  esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
  esp_bluedroid_init_with_cfg(new esp_bluedroid_config_t{{false}});
  esp_bluedroid_enable();
  esp_bt_dev_set_device_name("myBluetooth");
  esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
  esp_spp_register_callback(
      [](esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
      {
      switch (event)
      {
      case ESP_SPP_INIT_EVT:
      {
        esp_spp_start_srv(ESP_SPP_SEC_NONE,ESP_SPP_ROLE_SLAVE,0,"");
        break;
        }
      case ESP_SPP_DATA_IND_EVT:
      {
        
        char* data = new char[param->data_ind.len+1]{0};
        std::copy(param->data_ind.data,param->data_ind.data+param->data_ind.len,data);
        ESP_LOGI("BLT","data: %s",data);
        delete[] data;
        break;
      }
      default:
        break;
} });
  esp_spp_enhanced_init(new esp_spp_cfg_t{ESP_SPP_MODE_CB, false, 0});
  esp_bt_gap_set_pin(ESP_BT_PIN_TYPE_FIXED, 4, new esp_bt_pin_code_t{'1', '2', '3', '4'});
}

void main(void){
    startBluetooth();
}}