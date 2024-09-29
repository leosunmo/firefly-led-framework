#pragma once
#include "../Pattern.h"
#include "../../kylarLEDs/Utility/Timing.h"
#include "../../Effects/SoundReactive/RaindropEffect.h"
class Raindrop : public Pattern{

    public:
        using Pattern::Pattern;
        void init();
        void run();
        void release();
        double global_brightness = 1.0;
    private:
        Timing *myTiming;
        int startOffset = 9; // skip this many LEDs from the start
        int centerOffset = 0; // adjust the center point by this many LEDs
        int endOffset = 0;
        int midpoint;
        bool prototype = true; // use on the prototype to account for LED strip entry point
        RaindropEffect* rightRaindropEffect;
        RaindropEffect* leftRaindropEffect;
};