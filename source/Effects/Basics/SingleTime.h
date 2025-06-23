#pragma once
#include "../Effect.h"
#include "../../kylarLEDs/Utility/Timing.h"
//Single LED, time-based brightness

typedef struct{
    uint16_t index;
    float hue;
    float saturation;
    float brightness;
    uint16_t Toffset;
    uint16_t Trise;
    uint16_t Thold;
    uint16_t Tfall;
    float exp_dropoff;
} single_time_t; // Times are in milliseconds

class SingleTime : public Effect{

    public:
        using Effect::Effect;
        ~SingleTime();
        void run();
        void init();
        SingleTime* init(single_time_t blueprint);
    private:
        single_time_t profile;
        Timing *timer = NULL;

};


/*
x = time (milliseconds)
y = brightness    Trise     Thold
    |     |->       ->|->     ->|
1   |                xxxxxxxxxxx
    |              xx           xx
    |            xx               xx
0.5 |          xx                   xx
    |        xx                       xx
    |      xx                           xx
0   |xxxxxx                               xx___
    |   ->|                    |->       ->|
         Toffset                       Tfall


    |Toffset->Trise->Thold->Tfall
    
*/