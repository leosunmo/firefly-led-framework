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
        ~FullBar();

        float micVal = 0;
        float hueAdd = 0;
        float brightness = 0;
    private:
        Timing *hueTimer;
        Timing *secTimer;
        Timing *brightnessTimer;
        std::vector<SoundPixel*> *pixels;
};