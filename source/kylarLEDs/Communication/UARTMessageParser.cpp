#include "UARTMessageParser.h"
#include <string.h>
#include <stdio.h>

namespace FireFly {

UARTMessageParser::UARTMessageParser(MessageCallback callback) 
    : messageCallback(callback) {
    resetBuffer();
}

void UARTMessageParser::processByte(uint8_t byte) {
    // Print received byte for debugging
    printf("UART RX: 0x%02X (%c) [inMessage=%d, bufferIndex=%d]\n", 
           byte, (byte >= 32 && byte <= 126) ? byte : '.', inMessage, bufferIndex);
    
    // If we're not in a message and this byte is a start marker
    if (!inMessage && byte == MESSAGE_START) {
        printf("UART: Message start detected 0x%02X\n", byte);
        inMessage = true;
        buffer[0] = byte;
        bufferIndex = 1;
        return;
    }
    
    // If we're in a message
    if (inMessage) {
        // Check for buffer overflow
        if (bufferIndex >= BUFFER_SIZE) {
            printf("UART: Buffer overflow! Resetting.\n");
            resetBuffer();
            return;
        }
        
        // Add byte to buffer
        buffer[bufferIndex++] = byte;
        
        // Check if this is an end marker
        if (byte == MESSAGE_END) {
            printf("UART: Message end detected 0x%02X, length=%d\n", byte, bufferIndex);
            // If we have a complete message, parse it
            if (bufferIndex >= sizeof(UARTMessage)) {
                MessageStatus status = parseMessage();
                if (status != MessageStatus::SUCCESS) {
                    printf("UART: Parse error: %d\n", static_cast<int>(status));
                } else {
                    printf("UART: Message parsed successfully!\n");
                }
            } else {
                printf("UART: Message too short (len=%d, expected=%d)\n", 
                       bufferIndex, sizeof(UARTMessage));
            }
            resetBuffer();
        } else if (bufferIndex == sizeof(UARTMessage)) {
            printf("UART: Expected end marker but got 0x%02X\n", byte);
        }
    }
}

void UARTMessageParser::processBytes(const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        processByte(data[i]);
    }
}

MessageStatus UARTMessageParser::parseMessage() {
    if (bufferIndex < sizeof(UARTMessage)) {
        printf("UART parse: Message too short (%d < %d)\n", bufferIndex, sizeof(UARTMessage));
        return MessageStatus::INCOMPLETE;
    }
    
    UARTMessage message;
    memcpy(&message, buffer, sizeof(UARTMessage));
    
    // Validate message format
    if (message.start != MESSAGE_START) {
        printf("UART parse: Invalid start marker: 0x%02X != 0x%02X\n", 
               message.start, MESSAGE_START);
        return MessageStatus::INVALID_FORMAT;
    }
    
    if (message.end != MESSAGE_END) {
        printf("UART parse: Invalid end marker: 0x%02X != 0x%02X\n", 
               message.end, MESSAGE_END);
        return MessageStatus::INVALID_FORMAT;
    }
    
    // Print message details
    printf("UART parse: Valid message! Type=0x%02X, Value=0x%08X\n", 
           static_cast<uint8_t>(message.cmdType), message.value);
    
    // Call the callback with the valid message
    if (messageCallback) {
        messageCallback(message);
    } else {
        printf("UART parse: Warning - No callback registered!\n");
    }
    
    return MessageStatus::SUCCESS;
}

void UARTMessageParser::resetBuffer() {
    bufferIndex = 0;
    inMessage = false;
}

} // namespace FireFly
