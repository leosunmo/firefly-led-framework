#pragma once

#include <functional>
#include <vector>
#include <map>
#include <set>
#include "../Controllers/Sensors/Encoder/Encoder.h"
#include "../Controllers/Sensors/Button/Button.h"
#include "../Communication/UARTMessages.h"

namespace FireFly {

// Define input types that can come from either hardware or UART
enum class InputType {
    ENCODER_ROTATION,  // For any rotary encoder
    BUTTON_PRESS,      // For any button press
    BUTTON_RELEASE,    // For any button release
    VALUE_CHANGE,      // For direct value changes (like potentiometer or UART value)
};

// The logical event types that subscribers care about
enum class InputEventType {
    HUE,                // Hue changes (from either hw encoder or UART)
    BRIGHTNESS,         // Brightness changes
    PATTERN,            // Pattern selection changes
    EFFECT_PUNCH,       // Effect punch input, 0-1 button input
    SPEED,              // Speed control changes,  0-100 pot input
    CUSTOM_PARAM_1,     // Custom parameter 1
    CUSTOM_PARAM_2,     // Custom parameter 2
    CUSTOM_PARAM_3,     // Custom parameter 3
    
    COUNT               // Special enum to find last value
};

// Input event structure
struct InputEvent {
    InputType type;              // How the input occurred (encoder, button, value)
    InputEventType eventType;    // What parameter was affected
    int32_t value;               // Encoder count, button state, or normalized value (0-1000)
    int inputId;                 // Optional ID for debugging (auto-generated)
};

typedef std::function<void(const InputEvent&)> InputCallback;

class InputManager {
public:
    static InputManager& getInstance();
    
    // Register hardware inputs with their corresponding event types
    int registerEncoder(InputEventType eventType, Encoder* encoder);
    int registerButton(InputEventType eventType, Button* button);
    
    // Process UART messages into input events
    void processUARTMessage(const UARTMessage& msg);
    
    // Subscribe to input events by event type
    void subscribe(InputEventType eventType, InputCallback callback);
    
    // Unsubscribe from input events
    void unsubscribeAll(InputEventType eventType);
    
    // For direct access to values (rather than event callbacks)
    int32_t getValue(InputEventType eventType) const;
    
private:
    InputManager(); // Singleton
    
    // Structure to track hardware inputs
    struct HardwareInput {
        int id;
        InputEventType eventType;
        union {
            Encoder* encoder;
            Button* button;
            void* ptr;  // Generic pointer for future expansion
        };
        InputType inputType;
    };
    
    // Maps to store registered inputs and callbacks
    std::vector<HardwareInput> hardwareInputs;
    std::map<int, int32_t> inputValues;      // By input ID
    std::map<InputEventType, int32_t> eventValues;
    std::multimap<InputEventType, InputCallback> callbacks;
    
    // Map UART command types to event types
    std::map<CommandType, InputEventType> uartCommandToEventTypeMap;
    
    // Map custom parameters to event types
    std::map<int, InputEventType> customParamToEventTypeMap;
    
    // Generate sequential IDs for hardware inputs
    int nextInputId = 1;
    
    // Trigger callbacks for an input event
    void triggerCallbacks(const InputEvent& event);
    
    // Hardware callback handlers
    void handleEncoderChange(int inputId, InputEventType eventType, int count);
    void handleButtonPress(int inputId, InputEventType eventType, int state);
};

} // namespace FireFly
