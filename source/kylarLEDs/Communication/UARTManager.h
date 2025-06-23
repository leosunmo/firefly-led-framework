#pragma once

#include "UARTMessages.h"
#include "UARTMessageParser.h"
#include "../Input/InputManager.h"
#include "hardware/uart.h"

namespace FireFly {

class UARTManager {
public:
    // Singleton pattern
    static UARTManager& getInstance();
    
    // Initialize UART communication
    void init(uart_inst_t* uart, uint baudRate, uint txPin, uint rxPin);
    
    // Process all available UART input
    void processInput();
    
    // Send a UARTMessage over UART
    void sendMessage(const UARTMessage& msg);
    
    // Create and send a message with specific command and value
    void sendCommand(CommandType cmdType, uint32_t value);
    
    // Set a callback for status LED activity indication
    void setStatusLEDCallback(std::function<void(uint8_t)> callback) { statusLEDCallback = callback; }    

private:
    UARTManager(); // Private constructor (singleton)
    ~UARTManager();
    
    uart_inst_t* uartInst = nullptr;
    UARTMessageParser* parser = nullptr;
    std::function<void(uint8_t)> statusLEDCallback = nullptr;
    
    // Handle a message that was parsed from UART
    static void handleMessage(const UARTMessage& msg);
};

} // namespace FireFly
