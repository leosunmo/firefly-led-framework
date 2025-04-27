#pragma once
#include "../Pattern.h"
#include "../../kylarLEDs/Utility/Timing.h"
#include "../../Effects/SoundReactive/FullBar.h"
#include "../../kylarLEDs/Controllers/Sensors/Encoder/Encoder.h"
#include "../../kylarLEDs/Controllers/Sensors/Button/Button.h"
#include <vector>

class ShakeelFlash : public Pattern{

    public:
        ShakeelFlash(Encoder *effectEncoder, Button *effectButton) : Pattern(), effectEncoder(effectEncoder), effectButton(effectButton) {}
        using Pattern::Pattern;
        void run();
        void init();
        void release();

        Encoder *effectEncoder;
        Button *effectButton;
    private:
        double baseBrightness = 0.9; // Adjust this value to set the minimum brightness level
        FullBar * bar;
};