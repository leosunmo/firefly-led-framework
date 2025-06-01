#pragma once

#include <stdint.h>

namespace FireFly {

// Message start/end markers for framing
constexpr uint8_t MESSAGE_START = 0xFA;
constexpr uint8_t MESSAGE_END = 0xFB;

// Command types
enum class CommandType : uint8_t {
    BRIGHTNESS = 0x01,      // Set brightness value
    PATTERN = 0x02,         // Change pattern
    HUE = 0x03,             // Set hue value
    SATURATION = 0x04,      // Set saturation
    MODE = 0x05,            // Set operating mode
    SPEED = 0x06,           // Set animation speed
    POWER = 0x07,           // Power on/off
    GET_STATUS = 0x08,      // Request status
    CUSTOM_PARAM = 0x09,    // Custom parameter (top byte indicates param number 1-3)
    DEBUG = 0xFF,           // Debug message
};

// Message structure (7 bytes total)
struct __attribute__((packed)) UARTMessage {
    uint8_t start;          // MESSAGE_START marker
    CommandType cmdType;    // Command type
    uint32_t value;         // Command value (4 bytes for flexibility)
    uint8_t end;            // MESSAGE_END marker
};

// Status enum for parser results
enum class MessageStatus {
    SUCCESS,
    INCOMPLETE,
    INVALID_FORMAT,
    BUFFER_OVERFLOW
};

} // namespace FireFly
