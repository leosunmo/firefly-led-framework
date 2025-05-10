#include "ExampleESPNOWSender.h"
#include "esp_log.h"
#include "esp_now.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>
#include <cstdlib>

static const char *TAG = "ExampleESPNOWSender";

static QueueHandle_t exampleQueue = nullptr;

void ExampleESPNOWSender::init() {
    ESP_LOGI(TAG, "Initializing Example ESPNOW Sender");

    // Create a queue for ESPNOW events
    exampleQueue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(ESPNOWEvent));
    if (!exampleQueue) {
        ESP_LOGE(TAG, "Failed to create queue");
        return;
    }

    // Initialize ESPNOW
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(sendCallback));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(recvCallback));

    // Add broadcast peer
    if (!esp_now_is_peer_exist(broadcastMac)) {
        esp_now_peer_info_t peerInfo = {};
        peerInfo.channel = CONFIG_ESPNOW_CHANNEL;
        peerInfo.ifidx = static_cast<wifi_interface_t>(ESPNOW_WIFI_IF);
        peerInfo.encrypt = false;
        std::memcpy(peerInfo.peer_addr, broadcastMac, ESP_NOW_ETH_ALEN);
        ESP_ERROR_CHECK(esp_now_add_peer(&peerInfo));
    } else {
        ESP_LOGW(TAG, "Broadcast peer already exists");
    }

    // Start the example task
    xTaskCreate(exampleTask, "exampleTask", 2048, nullptr, 4, nullptr);
}

void ExampleESPNOWSender::sendCallback(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (!mac_addr) {
        ESP_LOGE(TAG, "Send callback error: null MAC address");
        return;
    }
    ESP_LOGI(TAG, "Send callback: MAC=%02x:%02x:%02x:%02x:%02x:%02x, status=%d",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], status);
}

void ExampleESPNOWSender::recvCallback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (!recv_info || !data || len <= 0) {
        ESP_LOGE(TAG, "Receive callback error: invalid arguments");
        return;
    }
    ESP_LOGI(TAG, "Receive callback: MAC=%02x:%02x:%02x:%02x:%02x:%02x, len=%d",
             recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2],
             recv_info->src_addr[3], recv_info->src_addr[4], recv_info->src_addr[5], len);
}

void ExampleESPNOWSender::exampleTask(void *pvParameter) {
    ESP_LOGI(TAG, "Example task started");

    uint8_t data[250]; // Example data buffer
    memset(data, 0xAB, sizeof(data)); // Fill with example data

    while (true) {
        ESP_LOGI(TAG, "Sending example data");

        esp_err_t result = esp_now_send(broadcastMac, data, sizeof(data));
        if (result == ESP_OK) {
            ESP_LOGI(TAG, "Data sent successfully");
        } else {
            ESP_LOGE(TAG, "Error sending data: %s", esp_err_to_name(result));
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    }
}