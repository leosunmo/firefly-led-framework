#pragma once
#include "../Pattern.h"
#include "../../kylarLEDs/Utility/Timing.h"
#include "../../Effects/SoundReactive/Spark.h"

class FireV0 : public Pattern{

    public:
        using Pattern::Pattern;
        
        void run();
        void init();
        
        void release();
        virtual ~FireV0();
        double global_brightness = 0;
        
    
    private:
        int initted = 0;
        const uint32_t num_sparks = 100;
        Spark* sparks[100];
        void create_new_sparks(int x_sparks);
        int calc_num_new_sparks();
        Timing *avgTimer;
        Timing *secTimer;
        Timing *valTimer;

        
        

};