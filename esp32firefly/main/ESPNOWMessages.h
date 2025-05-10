#ifndef ESPNOW_MESSAGES_H
#define ESPNOW_MESSAGES_H

#include <cstdint>
#include <memory>
#include <cstring>
#include "esp_now.h"
#include <variant>

struct ESPNOWEvent {
    enum class EventID {
        SEND_CB,
        RECV_CB
    } id;

    union {
        struct {
            uint8_t mac_addr[ESP_NOW_ETH_ALEN];
            esp_now_send_status_t status;
        } send_cb;

        struct {
            uint8_t mac_addr[ESP_NOW_ETH_ALEN];
            uint8_t *data;
            int data_len;
        } recv_cb;
    } info;
};

struct ESPNOWData {
    uint8_t type;       // Broadcast or unicast ESPNOW data.
    uint8_t state;      // Indicates if broadcast ESPNOW data has been received.
    uint16_t seq_num;   // Sequence number of ESPNOW data.
    uint16_t crc;       // CRC16 value of ESPNOW data.
    uint32_t magic;     // Magic number to determine which device to send unicast ESPNOW data.
    uint8_t payload[0]; // Real payload of ESPNOW data.
} __attribute__((packed));

struct SendParam {
    bool unicast;       // Send unicast ESPNOW data.
    bool broadcast;     // Send broadcast ESPNOW data.
    uint8_t state;      // Indicates if broadcast ESPNOW data has been received.
    uint32_t magic;     // Magic number to determine which device to send unicast ESPNOW data.
    uint16_t count;     // Total count of unicast ESPNOW data to be sent.
    uint16_t delay;     // Delay between sending two ESPNOW data, in ms.
    int len;            // Length of ESPNOW data to be sent, in bytes.
    std::unique_ptr<uint8_t[]> buffer; // Buffer pointing to ESPNOW data.
    uint8_t dest_mac[ESP_NOW_ETH_ALEN]; // MAC address of destination device.
};

#endif // ESPNOW_MESSAGES_H