#ifndef EXAMPLE_ESPNOW_SENDER_H
#define EXAMPLE_ESPNOW_SENDER_H

#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_now.h"
#include "ESPNOWMessages.h"
#include "ESPNOWManager.h"

class ExampleESPNOWSender {
public:
    static void init();

private:
    static void sendCallback(const uint8_t *mac_addr, esp_now_send_status_t status);
    static void recvCallback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len);
    static void exampleTask(void *pvParameter);

    static constexpr uint8_t broadcastMac[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
};

#endif // EXAMPLE_ESPNOW_SENDER_H