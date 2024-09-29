#pragma once
#include "../Effect.h"
#include "../Basics/SingleTime.h"
#include "../../kylarLEDs/Utility/Timing.h"
class Spark : public Effect{

    public:
        using Effect::Effect;
        ~Spark();
        void run();
        void reset(int position, int direction, float brightness, int steps, float hue, float hue_shift, float speed);
        void init();
        int8_t direction = 1;
        float micVal = 0;
        float hue = 0;
        float speed = 1;
        float position = 0;
        float hue_shift = 0;
        uint8_t num_steps = 0;
        uint8_t bonus_steps = 0;
        uint8_t bonused = 0;
        uint8_t total_steps = 0;
        single_time_t blueprint;
        float brightness_loss = 0.1;
        
    private:

        Timing *timer;
};