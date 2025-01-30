#pragma once
#include "../Pattern.h"
#include "../../kylarLEDs/Utility/Timing.h"
#include "../../kylarLEDs/Controllers/Sensors/Encoder/Encoder.h"
#include "../../kylarLEDs/Controllers/Sensors/Button/Button.h"
#include "../../Effects/SoundReactive/SoundPixel.h"
#include <vector>

struct BounceConfig
{
    bool solidBar;
    bool splitBar;
    bool highLowInput;
    int maxThickness = 5;
};

class Bounce : public Pattern
{
    BounceConfig config;

public:
    Bounce(const BounceConfig &config, Encoder *effectEncoder, Button *effectButton) : Pattern(), config(config), effectEncoder(effectEncoder),effectButton(effectButton) {}
    using Pattern::Pattern;
    void run();
    void init();
    void release();

    static int maxAllowedThickness;
    Encoder *effectEncoder;
    Button *effectButton;

private:
    Timing *myTiming;
    Timing *avgTimer;
    Timing *secTimer;
    Timing *valTimer;
    std::vector<SoundPixel *> *pixels;
};