#pragma once
#include "../Pattern.h"
#include "../../kylarLEDs/Utility/Timing.h"
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
    Bounce(const BounceConfig &config) : Pattern(), config(config) {}
    using Pattern::Pattern;
    void run();
    void init();
    void release();

private:
    Timing *myTiming;
    Timing *avgTimer;
    Timing *secTimer;
    Timing *valTimer;
    std::vector<SoundPixel *> *pixels;
};