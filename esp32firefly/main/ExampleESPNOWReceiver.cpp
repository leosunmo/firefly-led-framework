#include "ExampleESPNOWReceiver.h"
#include "esp_log.h"
#include "esp_now.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>
#include <cstdlib>
#include "ESPNOWMessages.h"
#include "ESPNOWManager.h"
#include "freertos/queue.h"
#include <vector>
#include <memory>
#include <new> // Include for placement new

static const char *TAG = "ExampleESPNOWReceiver";

static QueueHandle_t exampleQueue = nullptr;

void ExampleESPNOWReceiver::init() {
    ESP_LOGI(TAG, "Initializing Example ESPNOW Receiver");

    // Create a queue for ESPNOW events
    exampleQueue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(ESPNOWEvent));
    if (!exampleQueue) {
        ESP_LOGE(TAG, "Failed to create queue");
        return;
    }

    // Initialize ESPNOW
    esp_err_t err = esp_now_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ESPNOW: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "ESPNOW initialized successfully");

    // Register receive callback
    err = esp_now_register_recv_cb(recvCallback);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register receive callback: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "Receive callback registered successfully");

    // Start the example task
    xTaskCreate(exampleTask, "exampleTask", 2048, nullptr, 4, nullptr);
    ESP_LOGI(TAG, "Example receiver task started");
}

void ExampleESPNOWReceiver::recvCallback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (!recv_info || !data || len <= 0) {
        ESP_LOGE(TAG, "Receive callback error: invalid arguments");
        return;
    }

    ESP_LOGI(TAG, "Received ESPNOW data from MAC=%02x:%02x:%02x:%02x:%02x:%02x, len=%d",
             recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2],
             recv_info->src_addr[3], recv_info->src_addr[4], recv_info->src_addr[5], len);

    // Allocate memory for the event
    auto event = static_cast<ESPNOWEvent *>(malloc(sizeof(ESPNOWEvent)));
    if (!event) {
        ESP_LOGE(TAG, "Failed to allocate memory for ESPNOWEvent");
        return;
    }

    event->id = ESPNOWEvent::EventID::RECV_CB;
    std::memcpy(event->info.recv_cb.mac_addr, recv_info->src_addr, ESP_NOW_ETH_ALEN);
    event->info.recv_cb.data = static_cast<uint8_t *>(malloc(len));
    if (!event->info.recv_cb.data) {
        ESP_LOGE(TAG, "Failed to allocate memory for received data");
        free(event);
        return;
    }
    std::memcpy(event->info.recv_cb.data, data, len);
    event->info.recv_cb.data_len = len;

    // Send the event to the queue
    if (xQueueSend(exampleQueue, &event, portMAX_DELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to send event to queue");
        free(event->info.recv_cb.data);
        free(event);
    }
}

void ExampleESPNOWReceiver::exampleTask(void *pvParameter) {
    ESP_LOGI(TAG, "Receiver task running");

    while (true) {
        ESPNOWEvent *event;
        if (xQueueReceive(exampleQueue, &event, portMAX_DELAY) == pdTRUE) {
            if (event->id == ESPNOWEvent::EventID::RECV_CB) {
                ESP_LOGI(TAG, "Processing received data from MAC=%02x:%02x:%02x:%02x:%02x:%02x, len=%d",
                         event->info.recv_cb.mac_addr[0], event->info.recv_cb.mac_addr[1], event->info.recv_cb.mac_addr[2],
                         event->info.recv_cb.mac_addr[3], event->info.recv_cb.mac_addr[4], event->info.recv_cb.mac_addr[5], event->info.recv_cb.data_len);

                // Log the data content
                ESP_LOGD(TAG, "Data content: %.*s", event->info.recv_cb.data_len, event->info.recv_cb.data);

                // Free the allocated data
                free(event->info.recv_cb.data);
            }

            // Free the event structure
            free(event);
        }
    }
}