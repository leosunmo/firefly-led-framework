#pragma once
#include "../Pattern.h"
#include "../../kylarLEDs/Utility/Timing.h"
#include "../../Effects/SoundReactive/Spark.h"
#include "../../kylarLEDs/Utility/Waveforms/Triangle.h"

class FireV0 : public Pattern{
    bool useSound;

    public:
        FireV0(bool soundReactive) 
        : Pattern(), useSound(soundReactive) {
        // Constructor implementation for FireV0
    }
        
        void run();
        void init();
        
        void release();
        virtual ~FireV0();
        double global_brightness = 0;
        
    
    private:
        int initted = 0;
        static const uint32_t num_sparks = 80;
        Spark* sparks[num_sparks];
        void create_new_sparks(int x_sparks);
        int calc_num_new_sparks();
        Timing *avgTimer;
        Timing *secTimer;
        Timing *valTimer;
        const int num_triangles = 4;
        Triangle *triangles[4];

        
        

};