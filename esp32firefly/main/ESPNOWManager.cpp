#include "ESPNOWManager.h"
#include "esp_log.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <stdexcept>
#include <cstring>

static const char *TAG = "ESPNOWManager";

ESPNOWManager::ESPNOWManager() : queueHandle(nullptr) {}

ESPNOWManager::~ESPNOWManager() {
    // Clean up resources if necessary
}

esp_err_t ESPNOWManager::init() {
    esp_err_t err;

    err = initNVS();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(err));
        return err;
    }

    err = initWiFi();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WiFi: %s", esp_err_to_name(err));
        return err;
    }

    err = initESPNOW();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ESPNOW: %s", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}

void ESPNOWManager::deinitESPNOW() {
    if (queueHandle) {
        vQueueDelete(queueHandle);
        queueHandle = nullptr;
    }
    esp_now_deinit();
}

esp_err_t ESPNOWManager::initNVS() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

esp_err_t ESPNOWManager::initWiFi() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(ESPNOW_WIFI_MODE));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK(esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR));
#endif

    return ESP_OK;
}

esp_err_t ESPNOWManager::initESPNOW() {
    queueHandle = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(ESPNOWEvent));
    if (!queueHandle) {
        ESP_LOGE(TAG, "Failed to create queue");
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(sendCallback));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(recvCallback));

#if CONFIG_ESPNOW_ENABLE_POWER_SAVE
    ESP_ERROR_CHECK(esp_now_set_wake_window(CONFIG_ESPNOW_WAKE_WINDOW));
    ESP_ERROR_CHECK(esp_wifi_connectionless_module_set_wake_interval(CONFIG_ESPNOW_WAKE_INTERVAL));
#endif

    ESP_ERROR_CHECK(esp_now_set_pmk(reinterpret_cast<const uint8_t *>(CONFIG_ESPNOW_PMK)));

    esp_now_peer_info_t peerInfo = {};
    peerInfo.channel = CONFIG_ESPNOW_CHANNEL;
    peerInfo.ifidx = static_cast<wifi_interface_t>(ESPNOW_WIFI_IF);
    peerInfo.encrypt = false;
    std::memcpy(peerInfo.peer_addr, broadcastMac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK(esp_now_add_peer(&peerInfo));

    return ESP_OK;
}

void ESPNOWManager::sendCallback(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (!mac_addr) {
        ESP_LOGE(TAG, "Send callback error: null MAC address");
        return;
    }
    ESP_LOGI(TAG, "Send callback: MAC=%02x:%02x:%02x:%02x:%02x:%02x, status=%d",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], status);
}

void ESPNOWManager::recvCallback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (!recv_info || !data || len <= 0) {
        ESP_LOGE(TAG, "Receive callback error: invalid arguments");
        return;
    }
    ESP_LOGI(TAG, "Receive callback: MAC=%02x:%02x:%02x:%02x:%02x:%02x, len=%d",
             recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2],
             recv_info->src_addr[3], recv_info->src_addr[4], recv_info->src_addr[5], len);
}