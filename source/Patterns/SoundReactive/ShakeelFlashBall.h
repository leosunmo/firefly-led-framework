#pragma once
#include "../Pattern.h"
#include "../../kylarLEDs/Utility/Timing.h"
#include "../../Effects/SoundReactive/SoundPixel.h"
#include "../../Effects/SoundReactive/BounceBall.h"
#include <vector>
class ShakeelFlashBall : public Pattern{

    public:
        using Pattern::Pattern;
        void run();
        void init();
        void release();
    
    private:
        Timing *myTiming;
        int initted = 0;
        Timing *avgTimer;
        Timing *secTimer;
        Timing *valTimer;
        std::vector<SoundPixel*> *pixels;
        

};