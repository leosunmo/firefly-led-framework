#include "Flash.h"
#include <stdio.h>
#include <math.h>

/**
 * ==================================================================
 * Flash Pattern - Parameter and Mode Reference
 * ==================================================================
 * 
 * OVERVIEW
 * --------
 * Flash is a simple LED pattern that displays a solid color using the
 * SolidColor effect. It persists settings between instances and provides 
 * a user interface through the FireFly input system.
 * 
 * INTERACTIVE PARAMETERS
 * ---------------------
 * hue: double (0.0-1.0) - Color displayed by the pattern
 *      Controlled via HUE event with index=0
 *      Persists between pattern instances
 * 
 * brightness: float (0.0-1.0) - Brightness of the LEDs
 *      Default: 1.0 (full brightness)
 * 
 * saturation: float (0.0-1.0) - Color saturation
 *      Default: 1.0 (full saturation)
 * ==================================================================
 */

// Initialize the static settings with default values
FlashSettings Flash::persistentSettings;

Flash::Flash() : solidColorEffect(nullptr)
{
    // Load settings from persistent storage
    loadSettings();
}

// Load settings from persistent storage to local variables
void Flash::loadSettings()
{
    hue = persistentSettings.hue;
    saturation = persistentSettings.saturation;
    brightness = persistentSettings.brightness;
}

void Flash::init()
{
    // Ensure local variables are loaded from persistent settings
    loadSettings();
    
    // Create a new SolidColor effect
    solidColorEffect = new SolidColor();
    
    // Initialize the SolidColor effect
    solidColorEffect->init();
    
    // Apply the effect to the engine
    Effect::engine->apply(solidColorEffect);
    
    // Apply settings from persistent storage
    solidColorEffect->setHue(hue);
    solidColorEffect->setSaturation(saturation);
    solidColorEffect->setBrightness(brightness);
    
    // Set up input manager subscriptions
    auto& inputManager = FireFly::InputManager::getInstance();
    
    // Subscribe to HUE events
    inputManager.subscribe(FireFly::InputEventType::HUE, [this](const FireFly::InputEvent &event) {
        if (event.type == FireFly::InputType::VALUE_CHANGE) {
            // Convert the 0-360 degree value to 0-1 range
            double hueValue = event.value / 360.0;
            // Keep hue in 0-1 range
            if (hueValue > 1.0) hueValue = 1.0;
            if (hueValue < 0.0) hueValue = 0.0;
            
            // Only use index 0 for the hue
            if (event.index == 0) {
                printf("Flash - Setting hue: %f (index=%d)\n", hueValue, event.index);
                this->setHue(hueValue);
            } else {
                printf("Flash - Ignoring hue event with index: %d\n", event.index);
            }
        }
    });
    
    // Subscribe to BRIGHTNESS events
    inputManager.subscribe(FireFly::InputEventType::BRIGHTNESS, [this](const FireFly::InputEvent &event) {
        if (event.type == FireFly::InputType::VALUE_CHANGE) {
            // Brightness is a 0-100 value, map to 0-1
            float brightnessValue = event.value / 100.0f;
            
            printf("Flash - Setting brightness: %f\n", brightnessValue);
            this->setBrightness(brightnessValue);
        }
    });
}

void Flash::run()
{
    // Safety check
    if (!solidColorEffect) {
        return;
    }
    
    // The SolidColor effect handles everything automatically once initialized
    // No additional per-frame logic needed
}

void Flash::setHue(double h)
{
    // Store the hue
    hue = h;
    
    // Update persistent settings
    persistentSettings.hue = h;
    persistentSettings.userModified = true;
    
    if (solidColorEffect)
    {
        solidColorEffect->setHue(hue);
    }
}

void Flash::setSaturation(float s)
{
    // Store the saturation
    saturation = s;
    
    // Update persistent settings
    persistentSettings.saturation = s;
    persistentSettings.userModified = true;
    
    if (solidColorEffect)
    {
        solidColorEffect->setSaturation(saturation);
    }
}

void Flash::setBrightness(float b)
{
    // Store the brightness
    brightness = b;
    
    // Update persistent settings
    persistentSettings.brightness = b;
    persistentSettings.userModified = true;
    
    if (solidColorEffect)
    {
        solidColorEffect->setBrightness(brightness);
    }
}

const char* Flash::getName()
{
    return "Flash";
}

void Flash::release()
{
    // Set the solidColorEffect pointer to null after the effect is deleted by the EffectEngine
    // This prevents us from trying to access the deleted effect if we're reinitialized
    solidColorEffect = nullptr;
    
    // Unsubscribe from input events
    auto& inputManager = FireFly::InputManager::getInstance();
    inputManager.unsubscribeAll(FireFly::InputEventType::HUE);
    inputManager.unsubscribeAll(FireFly::InputEventType::BRIGHTNESS);
}
