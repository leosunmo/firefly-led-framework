#ifndef EXAMPLE_ESPNOW_RECEIVER_H
#define EXAMPLE_ESPNOW_RECEIVER_H

#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_now.h"
#include "ESPNOWMessages.h"

class ExampleESPNOWReceiver {
public:
    static void init();

private:
    static void recvCallback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len);
    static void exampleTask(void *pvParameter);
};

#endif // EXAMPLE_ESPNOW_RECEIVER_H