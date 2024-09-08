#pragma once
#include "../Effect.h"
#include <vector>
#include "../../kylarLEDs/Utility/Timing.h"
#include <stdio.h>
struct Pixel {
	int wateryness;
	bool moving;
	int led;
	Pixel(int l) : wateryness(0), moving(false), led(l) {}
};

class RaindropEffect : public Effect{
    public:
        using Effect::Effect;
        std::vector<Pixel> *pixels;
        void run();
        void init();
        void move_pixel(int position);  
        ~RaindropEffect();
        double hue = 0;
    private:
        Timing *myTiming;
        int unused = 0;
        double previousMicValue = 0;
        double micValue = 0;
        double test = 0;
};