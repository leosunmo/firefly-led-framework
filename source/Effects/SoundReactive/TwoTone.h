#pragma once
#include "../Effect.h"
#include "../../kylarLEDs/Utility/Timing.h"
#include "../../Effects/SoundReactive/SoundPixel.h"
#include <vector>

class TwoTone : public Effect {
    
public:
    using Effect::Effect;
    void run();
    void init();
    void resetSecTimer();

    ~TwoTone();

    float micVal = 0;
    float smoothingMicVal = 50;
    float hueAdd = 0;
    
    // Adjust this value to set the baseline brightness level.
    // Maximum should probably be around 0.95.
    // Minimum should be around 0.05.
    double baseBrightness = 0.9;
    float brightness = 0;
    
    // Set the two hues to cycle between
    // hue1, hue2: Hue values in the range [0,1] where:
    //   0.0 = red, 0.33 = green, 0.66 = blue
    void setHues(float hue1, float hue2);
    
    // First hue in the cycle
    float getHue1() const { return hue1; }
    void setHue1(float value);
    
    // Second hue in the cycle
    float getHue2() const { return hue2; }
    void setHue2(float value);
    
    // Set pulse period scaling factor (lower = faster response)
    // This affects how quickly the effect responds to audio
    float getPeriod() const { return pulsePeriod; }
    void setPeriod(float seconds);
    
    // Set pulse reactivity (recommended range: 1.0-3.0)
    // Higher values make the effect more responsive to audio
    float getReactivity() const { return pulseReactivity; }
    void setReactivity(float value);
        
private:
    Timing *hueTimer;
    Timing *secTimer;
    Timing *brightnessTimer;
    std::vector<SoundPixel*> *pixels;
    
    // Pulse transition position (0 to 1)
    float pulsePosition = 0.0f;
    
    // Two-hue pulse parameters
    float hue1 = 0.0f;             // First hue (0-1.0)
    float hue2 = 0.66f;            // Second hue (0-1.0) 
    float pulsePeriod = 1.0f;      // Period scaling (lower = faster response)
    float pulseReactivity = 2.0f;  // Audio reactivity (higher = more responsive)
};
