#include "InputManager.h"
#include <stdio.h>

namespace FireFly {

InputManager& InputManager::getInstance() {
    static InputManager instance;
    return instance;
}

InputManager::InputManager() {
    // Initialize all event values to 0
    for (int i = 0; i < static_cast<int>(InputEventType::COUNT); i++) {
        eventValues[static_cast<InputEventType>(i)] = 0;
    }
    
    // Set up UART command to event type mappings
    uartCommandToEventTypeMap[CommandType::HUE] = InputEventType::HUE;
    uartCommandToEventTypeMap[CommandType::BRIGHTNESS] = InputEventType::BRIGHTNESS;
    uartCommandToEventTypeMap[CommandType::PATTERN] = InputEventType::PATTERN;
    uartCommandToEventTypeMap[CommandType::SPEED] = InputEventType::SPEED;
    uartCommandToEventTypeMap[CommandType::PUNCH] = InputEventType::EFFECT_PUNCH;

    
    // Set up custom parameter mappings
    customParamToEventTypeMap[1] = InputEventType::CUSTOM_PARAM_1;
    customParamToEventTypeMap[2] = InputEventType::CUSTOM_PARAM_2;
    customParamToEventTypeMap[3] = InputEventType::CUSTOM_PARAM_3;
}

int InputManager::registerEncoder(InputEventType eventType, Encoder* encoder) {
    if (encoder) {
        int id = nextInputId++;
        
        // Store the encoder in our list
        HardwareInput input = {
            .id = id,
            .eventType = eventType,
            .encoder = encoder,
            .inputType = InputType::ENCODER_ROTATION
        };
        hardwareInputs.push_back(input);
        
        // Set up encoder callback
        encoder->setCallback([this, id, eventType](int count) {
            this->handleEncoderChange(id, eventType, count);
        });
        
        return id;
    }
    return 0;
}

int InputManager::registerButton(InputEventType eventType, Button* button) {
    if (button) {
        int id = nextInputId++;
        
        // Store the button in our list
        HardwareInput input = {
            .id = id,
            .eventType = eventType,
            .button = button,
            .inputType = InputType::BUTTON_PRESS
        };
        hardwareInputs.push_back(input);
        
        // Set up button callback with press state parameter
        button->setCallback([this, id, eventType](int state) {
            this->handleButtonPress(id, eventType, state);
        });
        
        return id;
    }
    return 0;
}

void InputManager::handleEncoderChange(int inputId, InputEventType eventType, int count) {
    // Update input value
    inputValues[inputId] = count;
    
    // Update event type value
    eventValues[eventType] = count;
    
    // Create and trigger event
    InputEvent event = {
        .type = InputType::ENCODER_ROTATION,
        .eventType = eventType,
        .value = count,
        .inputId = inputId,
        .index = 0
    };
    
    triggerCallbacks(event);
}

void InputManager::handleButtonPress(int inputId, InputEventType eventType, int state) {
    // State is directly from the Button class:
    // - state=1 means button is pressed (connected to ground)
    // - state=0 means button is released (pulled up)
    inputValues[inputId] = state;
    
    // Update event type value with the button state
    eventValues[eventType] = state;
    
    // Create and trigger event with the appropriate input type
    InputEvent event = {
        .type = state ? InputType::BUTTON_PRESS : InputType::BUTTON_RELEASE,
        .eventType = eventType,
        .value = state,
        .inputId = inputId,
        .index = 0
    };
    
    triggerCallbacks(event);
}

void InputManager::processUARTMessage(const UARTMessage& msg) {
    InputType type = InputType::VALUE_CHANGE;
    int32_t value = static_cast<int32_t>(msg.value);
    InputEventType eventType;
    bool validCommand = true;
    
    // Map UARTMessage command types to event types
    switch (msg.cmdType) {
        case CommandType::HUE: {
            // Extract the index from the upper 16 bits and the hue value from the lower 16 bits
            uint8_t index = (msg.value >> 16) & 0xFF;
            uint16_t hueValue = msg.value & 0xFFFF;
            
            auto it = uartCommandToEventTypeMap.find(msg.cmdType);
            if (it != uartCommandToEventTypeMap.end()) {
                eventType = it->second;
                value = static_cast<int32_t>(hueValue);
                printf("UART Hue command received: index=%d, hue=%d°\n", index, hueValue);
            } else {
                printf("Error: No event type mapping for UART command type %d\n", 
                       static_cast<int>(msg.cmdType));
                validCommand = false;
            }
            break;
        }
        case CommandType::BRIGHTNESS:
        case CommandType::PUNCH:
        case CommandType::SPEED: {
            auto it = uartCommandToEventTypeMap.find(msg.cmdType);
            if (it != uartCommandToEventTypeMap.end()) {
                eventType = it->second;
            } else {
                printf("Error: No event type mapping for UART command type %d\n", 
                       static_cast<int>(msg.cmdType));
                validCommand = false;
            }
            break;
        }
        
        case CommandType::PATTERN: {
            // Special handling for PATTERN command to directly select a pattern
            auto it = uartCommandToEventTypeMap.find(msg.cmdType);
            if (it != uartCommandToEventTypeMap.end()) {
                eventType = it->second;
                printf("UART Pattern command received: switching to pattern %d\n", value);
            } else {
                printf("Error: No event type mapping for UART command type %d\n", 
                       static_cast<int>(msg.cmdType));
                validCommand = false;
            }
            break;
        }
            
        case CommandType::CUSTOM_PARAM: {
            uint8_t paramNum = (msg.value >> 24) & 0xFF;
            uint32_t paramValue = msg.value & 0xFFFFFF;
            
            auto it = customParamToEventTypeMap.find(paramNum);
            if (it != customParamToEventTypeMap.end()) {
                eventType = it->second;
                value = static_cast<int32_t>(paramValue);
            } else {
                printf("Error: No event type mapping for custom parameter %d\n", paramNum);
                validCommand = false;
            }
            break;
        }
            
        default:
            printf("Unknown UART command type: %d\n", static_cast<int>(msg.cmdType));
            validCommand = false;
    }
    
    if (validCommand) {
        // Update the event value
        eventValues[eventType] = value;
        
        // For button-related event types, use button press/release input types
        if ((isButtonEventType(eventType) || msg.cmdType == CommandType::PUNCH) && msg.cmdType != CommandType::PATTERN) {
            // PATTERN is excluded from this logic when sent via UART to enable direct pattern selection
            // Interpret value as button state: 1=pressed, 0=released
            bool isPressed = (value != 0);
            type = isPressed ? InputType::BUTTON_PRESS : InputType::BUTTON_RELEASE;
            value = isPressed ? 1 : 0; // Normalize to 1 or 0
        }
        
        // Create and trigger event
        uint8_t hueIndex = 0;
        if (msg.cmdType == CommandType::HUE) {
            hueIndex = static_cast<uint8_t>((msg.value >> 16) & 0xFF);
        }
        
        InputEvent event = {
            .type = type,
            .eventType = eventType,
            .value = value,
            .inputId = 0,  // 0 indicates UART source
            .index = hueIndex
        };
        
        triggerCallbacks(event);
    }
}

void InputManager::subscribe(InputEventType eventType, InputCallback callback) {
    if (callback) {
        callbacks.insert(std::make_pair(eventType, callback));
    }
}

void InputManager::unsubscribeAll(InputEventType eventType) {
    callbacks.erase(eventType);
}

void InputManager::triggerCallbacks(const InputEvent& event) {
    auto range = callbacks.equal_range(event.eventType);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second) {
            it->second(event);
        }
    }
}

int32_t InputManager::getValue(InputEventType eventType) const {
    auto it = eventValues.find(eventType);
    if (it != eventValues.end()) {
        return it->second;
    }
    return 0;
}

bool InputManager::isButtonEventType(InputEventType eventType) const {
    // PATTERN can be either a button press or a direct value selection depending on the source
    // EFFECT_PUNCH is always a button interaction
    return (eventType == InputEventType::EFFECT_PUNCH);
    
    // Add any other button event types as needed
}

} // namespace FireFly
