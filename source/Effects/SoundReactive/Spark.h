#pragma once
#include "../Effect.h"
#include "../Basics/SingleTime.h"
#include "../../kylarLEDs/Utility/Timing.h"
class Spark : public Effect{

    public:
        using Effect::Effect;
        ~Spark();
        void run();
        void reset(int position, int direction, double brightness, int steps, double hue, double hue_shift, double speed);
        void init();
        int direction = 1;
        float micVal = 0;
        double hue = 0;
        double speed = 1;
        double position = 0;
        double hue_shift = 0;
        int num_steps = 0;
        single_time_t blueprint;
        double brightness_loss = 0.1;
    private:
        int lastPosition = 0;
        int nextPosition = 0;
        
        Timing *timer;
};