#include <stdio.h>
#include "bluetooth.h"
void app_main(void)
{
    // Khởi động Bluetooth
    startBluetooth();

    // Vòng lặp vô hạn (có thể thêm xử lý khác nếu cần)
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // Chờ 1 giây
    }
}
