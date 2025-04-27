#pragma once
#include "../Pattern.h"
#include "../../kylarLEDs/Utility/Timing.h"
#include "../../Effects/SoundReactive/FullBar.h"
#include <vector>
class ShakeelFlash : public Pattern{

    public:
        using Pattern::Pattern;
        void run();
        void init();
        void release();

    private:
        FullBar * bar;
};