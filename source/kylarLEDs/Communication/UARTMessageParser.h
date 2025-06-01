#pragma once

#include "UARTMessages.h"
#include <functional>

namespace FireFly {

class UARTMessageParser {
public:
    // Callback type for message handlers
    using MessageCallback = std::function<void(const UARTMessage&)>;

    UARTMessageParser(MessageCallback callback);
    
    // Process a single incoming byte
    void processByte(uint8_t byte);
    
    // Process multiple bytes at once
    void processBytes(const uint8_t* data, size_t length);

private:
    static constexpr size_t BUFFER_SIZE = 32;
    uint8_t buffer[BUFFER_SIZE];
    size_t bufferIndex = 0;
    bool inMessage = false;
    MessageCallback messageCallback;
    
    // Try to parse a complete message from the buffer
    MessageStatus parseMessage();
    
    // Reset buffer state
    void resetBuffer();
};

} // namespace FireFly
