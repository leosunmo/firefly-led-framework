#pragma once
#include "../Effect.h"
#include "../Basics/SingleTime.h"
#include "../../kylarLEDs/Utility/Timing.h"

class BounceBall : public Effect{

    public:
        using Effect::Effect;
        void run();
        void init();
        ~BounceBall();
        void reset(int position, int direction, float brightness, float hue, float hue_shift, float speed);
        int direction = 1;
        float pos = 0; // Current index.
        float vel = 0; // Velocity.
        // Bouncing is currently broken, it doesn't stand still enough to get removed.
        // float bounce_factor = 0.7;
        float bounce_factor = 0;
        float hue = 0;
        uint16_t bounce_end = 0;
        float hue_shift;
        single_time_t blueprint;
        float gravity = 10; // Leds / s / s
        Timing * velTimer;
};