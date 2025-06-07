#pragma once
#include "../Pattern.h"
#include "../../kylarLEDs/Utility/Timing.h"
#include "../../kylarLEDs/Controllers/Sensors/Encoder/Encoder.h"
#include "../../kylarLEDs/Controllers/Sensors/Button/Button.h"
#include "../../Effects/SoundReactive/SoundPixel.h"
#include "../../Effects/SoundReactive/BounceBall.h"
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
    Bounce(BounceConfig config) : config(config) {}
    using Pattern::Pattern;
    void run();
    void init();
    void release();

    static int maxAllowedThickness;

private:
    Timing *avgTimer;
    Timing *secTimer;
    Timing *valTimer;
    std::vector<SoundPixel *> *pixels;
};