#pragma once
#include "../Pattern.h"
#include "../../Effects/Basics/SolidColor.h"
#include "../../kylarLEDs/Input/InputManager.h"

// Structure to hold persistent settings for Flash pattern
struct FlashSettings {
    double hue = 0; // Hue doesn't matter for white
    float saturation = 0.0f; // No saturation for white
    float brightness = 1.0f; // Full brightness
    
    // Tracks whether a setting has been modified by user
    bool userModified = false;
};

/**
 * Flash Pattern
 * 
 * A simple pattern that displays a solid color using the SolidColor effect.
 * The hue can be changed through the InputManager's HUE events.
 */
class Flash : public Pattern {
public:
    Flash();
    void run() override;
    void init() override;
    void release() override;
    
    // Returns the name of the pattern
    const char* getName();
    
    // Set parameters
    void setHue(double h);
    void setSaturation(float s);
    void setBrightness(float b);
    
private:
    // The solid color effect used by this pattern
    SolidColor* solidColorEffect;
    
    // Settings that persist between instances
    static FlashSettings persistentSettings;
    
    // Local copies of settings
    double hue;
    float saturation;
    float brightness;
    
    // Load settings from persistent storage
    void loadSettings();
};
