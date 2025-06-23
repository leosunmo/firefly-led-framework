#pragma once
#include "../Effect.h"
#include "../../kylarLEDs/Utility/Timing.h"
#include "../../Effects/SoundReactive/SoundPixel.h"
#include <vector>

class FullBar : public Effect{
    
    public:
        using Effect::Effect;
        void run();
        void init();
        void resetSecTimer();

        ~FullBar();

        float micVal = 0;

        float smoothingMicVal = 50;

        float hueAdd = 0;
        // Adjust this value to set the baseline brightness level.
        // Maximum should probably be around 0.95.
        // Minimum should be around 0.05.
        double baseBrightness = 0.9;
        float brightness = 0;
    private:
        Timing *hueTimer;
        Timing *secTimer;
        Timing *brightnessTimer;
        std::vector<SoundPixel*> *pixels;
};