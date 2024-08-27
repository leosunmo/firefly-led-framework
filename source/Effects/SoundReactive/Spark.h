#pragma once
#include "../Effect.h"
#include "../Basics/SingleTime.h"
#include "../../kylarLEDs/Utility/Timing.h"
class Spark : public Effect{

    public:
        using Effect::Effect;
        ~Spark();
        void run();
        void init();
        int direction = 1;
        float micVal = 0;
        double hue = 0;
        single_time_t blueprint;
    private:
        double speed = 1;
        double position = 0;
        int lastPosition = 0;
        int nextPosition = 0;
        
        Timing *timer;
};