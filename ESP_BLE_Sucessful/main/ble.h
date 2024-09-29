#ifndef __BLE_H
#define __BLE_H

#include <stdint.h>
#include <inttypes.h>
typedef void (*ble_data_recv_handle_t)(uint8_t *data, uint16_t length);
void app_ble_send_data(uint8_t* data, uint16_t len);
void app_ble_stop(void);
void app_ble_start(void);
void app_ble_set_data_recv_callback(void *cb);
#endif