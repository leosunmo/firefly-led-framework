#pragma once
#include "../Pattern.h"
#include "../../kylarLEDs/Utility/Timing.h"
#include "../../Effects/SoundReactive/FullBar.h"
#include "../../kylarLEDs/Input/InputManager.h"
#include <vector>

class ShakeelFlash : public Pattern{

    public:
        ShakeelFlash() : Pattern() {}

        void run();
        void init();
        void release();

        Encoder *effectEncoder;
        Button *effectButton;
    private:
        // punch is an effect booster that is triggered for a short time
        // when the button is pressed
        bool punch = false;
        Timing *punchTimer;
        double baseBrightness = 0.9; // Adjust this value to set the minimum brightness level
        FullBar * bar;
};