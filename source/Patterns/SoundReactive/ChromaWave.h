#pragma once
#include "../Pattern.h"
#include "../../Effects/SoundReactive/TwoTone.h"
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"
#include "../../kylarLEDs/Controllers/Controller.h"
#include "../../kylarLEDs/Input/InputManager.h"


class ChromaWave : public Pattern {
public:
    ChromaWave();
    void run() override;
    void init() override;
    void release() override;
    
    void setHue(double hue);
    void setBrightness(float b);
    void setSpeed(float speed);
    
    // Custom function to set the second hue
    void setSecondHue(double hue);
    
    // Custom functions for pulse parameters
    void setPeriod(float seconds);
    void setReactivity(float value);
    
    const char* getName();

private:
    TwoTone* twoToneEffect;
    double hue = 0;
    double hue2 = 0.66; // Default second hue (blue)
};
