#include "InputManager.h"
#include <stdio.h>

namespace FireFly {

InputManager& InputManager::getInstance() {
    static InputManager instance;
    return instance;
}

InputManager::InputManager() {
    // Initialize all current values to 0
    for (int i = 0; i < static_cast<int>(InputSource::COUNT); i++) {
        currentValues[static_cast<InputSource>(i)] = 0;
    }
}

void InputManager::registerEncoder(InputSource source, Encoder* encoder) {
    if (encoder) {
        encoders[source] = encoder;
        
        // Set up encoder callback
        encoder->setCallback([this, source](int count) {
            this->handleEncoderChange(source, count);
        });
    }
}

void InputManager::registerButton(InputSource source, Button* button) {
    if (button) {
        buttons[source] = button;
        
        // Set up button callback
        button->setCallback([this, source]() {
            this->handleButtonPress(source);
        });
    }
}

void InputManager::handleEncoderChange(InputSource source, int count) {
    // Update current value
    currentValues[source] = count;
    
    // Create and trigger event
    InputEvent event = {
        .type = InputType::ENCODER_ROTATION,
        .source = source,
        .value = count
    };
    
    triggerCallbacks(event);
}

void InputManager::handleButtonPress(InputSource source) {
    // Toggle button state (0 or 1)
    currentValues[source] = currentValues[source] ? 0 : 1;
    
    // Create and trigger event
    InputEvent event = {
        .type = InputType::BUTTON_PRESS,
        .source = source,
        .value = currentValues[source]
    };
    
    triggerCallbacks(event);
}

void InputManager::processUARTMessage(const UARTMessage& msg) {
    InputSource source;
    InputType type = InputType::VALUE_CHANGE;
    int32_t value = static_cast<int32_t>(msg.value);
    
    // Map UARTMessage command types to InputSources
    switch (msg.cmdType) {
        case CommandType::BRIGHTNESS:
            source = InputSource::UART_BRIGHTNESS;
            break;
            
        case CommandType::HUE:
            source = InputSource::UART_HUE;
            break;
            
        case CommandType::PATTERN:
            source = InputSource::UART_PATTERN;
            break;
            
        case CommandType::SPEED:
            source = InputSource::UART_SPEED;
            break;
            
        case CommandType::CUSTOM_PARAM: {
            // Handle custom parameters in a block scope to isolate variable declarations
            uint8_t paramNum = (msg.value >> 24) & 0xFF;
            uint32_t paramValue = msg.value & 0xFFFFFF;
            
            if (paramNum == 1) source = InputSource::UART_CUSTOM_PARAM_1;
            else if (paramNum == 2) source = InputSource::UART_CUSTOM_PARAM_2;
            else source = InputSource::UART_CUSTOM_PARAM_3;
            
            // Update the value to use the extracted parameter value
            value = static_cast<int32_t>(paramValue);
            
            // Update current value and trigger callbacks
            currentValues[source] = value;
            
            // Create and trigger event
            InputEvent event = {
                .type = type,
                .source = source,
                .value = value
            };
            
            triggerCallbacks(event);
            return;  // We've already handled this special case
        }
            
        default:
            printf("Unknown UART command type: %d\n", static_cast<int>(msg.cmdType));
            return;
    }
    
    // Create and trigger event
    InputEvent event = {
        .type = type,
        .source = source,
        .value = value
    };
    
    // Update current value
    currentValues[source] = value;
    
    // Trigger callbacks
    triggerCallbacks(event);
}

void InputManager::subscribe(InputSource source, InputCallback callback) {
    if (callback) {
        callbacks.insert(std::make_pair(source, callback));
    }
}

void InputManager::unsubscribeAll(InputSource source) {
    callbacks.erase(source);
}

void InputManager::triggerCallbacks(const InputEvent& event) {
    auto range = callbacks.equal_range(event.source);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second) {
            it->second(event);
        }
    }
}

int32_t InputManager::getValue(InputSource source) const {
    auto it = currentValues.find(source);
    if (it != currentValues.end()) {
        return it->second;
    }
    return 0;
}

} // namespace FireFly