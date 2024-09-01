#pragma once
#include "../Effect.h"

class Gradient : public Effect{

    public:
        using Effect::Effect;
        void run();
        void init();
        ~Gradient();
        int start = 0;
        int stop = NUM_LEDS;
        double start_hue = 0;
        double stop_hue = 1;
        double brightness = 0.4;
        double last_value[NUM_LEDS];
};