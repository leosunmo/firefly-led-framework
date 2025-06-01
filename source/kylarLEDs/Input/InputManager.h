#pragma once

#include <functional>
#include <vector>
#include <map>
#include "../Controllers/Sensors/Encoder/Encoder.h"
#include "../Controllers/Sensors/Button/Button.h"
#include "../Communication/UARTMessages.h"

namespace FireFly {

// Define input types that can come from either hardware or UART
enum class InputType {
    ENCODER_ROTATION,  // For any rotary encoder
    BUTTON_PRESS,      // For any button press
    VALUE_CHANGE,      // For direct value changes (like potentiometer or UART value)
};

// Define specific input sources (both hardware and virtual)
enum class InputSource {
    // Hardware inputs
    HW_HUE_ENCODER,
    HW_BRIGHTNESS_ENCODER, 
    HW_EFFECT_ENCODER,
    HW_PATTERN_BUTTON,
    HW_EFFECT_BUTTON,
    HW_POTENTIOMETER,
    
    // Virtual inputs (from UART)
    UART_HUE,
    UART_BRIGHTNESS,
    UART_EFFECT_PARAM,
    UART_PATTERN,
    UART_SPEED,
    UART_CUSTOM_PARAM_1,
    UART_CUSTOM_PARAM_2,
    UART_CUSTOM_PARAM_3,

    // Special enum to find last value of enum for iteration
    // Always keep this as the last enum value
    COUNT
};

// Input event structure
struct InputEvent {
    InputType type;
    InputSource source;
    int32_t value;      // Encoder count, button state, or normalized value (0-1000)
};

typedef std::function<void(const InputEvent&)> InputCallback;

class InputManager {
public:
    static InputManager& getInstance();
    
    // Register hardware inputs
    void registerEncoder(InputSource source, Encoder* encoder);
    void registerButton(InputSource source, Button* button);
    
    // Process UART messages into input events
    void processUARTMessage(const UARTMessage& msg);
    
    // Subscribe to input events
    void subscribe(InputSource source, InputCallback callback);
    
    // Unsubscribe from input events
    void unsubscribeAll(InputSource source);
    
    // For direct access to values (rather than event callbacks)
    int32_t getValue(InputSource source) const;
    
private:
    InputManager(); // Singleton
    
    // Maps to store registered inputs and callbacks
    std::map<InputSource, Encoder*> encoders;
    std::map<InputSource, Button*> buttons;
    std::map<InputSource, int32_t> currentValues;
    std::multimap<InputSource, InputCallback> callbacks;
    
    // Trigger callbacks for an input event
    void triggerCallbacks(const InputEvent& event);
    
    // Hardware callback handlers
    void handleEncoderChange(InputSource source, int count);
    void handleButtonPress(InputSource source);
};

} // namespace FireFly