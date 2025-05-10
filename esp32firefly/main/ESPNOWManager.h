#ifndef ESPNOW_MANAGER_H
#define ESPNOW_MANAGER_H

#include <vector>
#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_now.h"
#include "ESPNOWMessages.h"

/* ESPNOW can work in both station and softap mode. It is configured in menuconfig. */
#if CONFIG_ESPNOW_WIFI_MODE_STATION
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_STA
#else
#define ESPNOW_WIFI_MODE WIFI_MODE_AP
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_AP
#endif

#define ESPNOW_QUEUE_SIZE 6

class ESPNOWManager {
public:
    ESPNOWManager();
    ~ESPNOWManager();

    esp_err_t init();

private:
    QueueHandle_t queueHandle;
    const uint8_t broadcastMac[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    esp_err_t initNVS();
    esp_err_t initWiFi();
    esp_err_t initESPNOW();
    void deinitESPNOW();

    static void sendCallback(const uint8_t *mac_addr, esp_now_send_status_t status);
    static void recvCallback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len);
};

#endif // ESPNOW_MANAGER_H