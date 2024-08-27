#include "FireV0.h"
#include <stdio.h>
#include "../../Effects/Basics/ExampleEffect.h"
#include "../../Effects/Effect.h"
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"



FireV0::~FireV0() {}

void FireV0::init(){
    // FireV0Logo *FireV0_logo = new FireV0Logo();
    // FireV0_logo->init();
    //Effect::engine->apply(FireV0_logo);
    for(int i = 0; i < num_sparks; i++){
        sparks[i] = new Spark();
        Effect::engine->queueApply(sparks[i]);
    }
    secTimer = new Timing();
    avgTimer = new Timing();
    valTimer = new Timing();
}

int FireV0::calc_num_new_sparks(){
    // Currently the Run takes care of this
}

void FireV0::create_new_sparks(int x_sparks){
    static int left_side = 0;
    static int right_side = NUM_LEDS - 1;
    static int side = 0; // Flip the side they're created on
    static int spark_index = 0; // Increment this
    static double spark_hue = 0.9;

    int spark_pos;
    int spark_dir;
    double brightness = 0.2;
    int steps = x_sparks;
    double speed = x_sparks / 10.0;
    for(int i = 0; i < x_sparks; i++){
        side = !side;
        spark_pos = side ? left_side : right_side;
        spark_dir = side ? 1 : -1;
        steps = std::min(i, NUM_LEDS/2 - 2);
        brightness = (i)/(double)x_sparks;
        Spark* spark = sparks[i];
        
        spark->reset(spark_pos,
                     spark_dir,
                     brightness,
                     steps,
                     spark_hue,
                     0.01 /*hue shift */,
                     speed + i*0.2);
        
        spark_hue += 0.01;
        if(spark_hue > 1.05){
            spark_hue = 1;
        }
        if(++spark_index >= num_sparks){
            spark_index = 0;
        }
    }
}

void FireV0::run(){
    double micVal = pow(Microphone::getLowNormal(),2) ;
    static double brightness = 0;
    static double highVal = 0;
    double result = 0;
    double seconds = secTimer->takeSeconds();
    int avgLoops = 0;
    // Color movement
    //printf("micTimer = %d ... %f\n", micTimer->timerMs(), micTimer->takeSeconds());

    // printf("avgTimer = %d ... %d\n", avgTimer->timerMs());//, avgTimer->takeMsEvery(25));
    avgLoops = avgTimer->takeMsEvery(1);


    // Brightness
    double proposedBrightness = (micVal);
    //printf("valTimer = %d ... %d\n", valTimer->timerMs(), valTimer->takeMsEvery(100));
    avgLoops = valTimer->takeMsEvery(1);
    for(int i = 0; i < avgLoops; i++){
        //brightness = (brightness*120.0 + proposedBrightness)/121.0;
        if(proposedBrightness > brightness){
            //increasing
            brightness = (brightness*120.0 + proposedBrightness) / 121.0;
        }else{
            //decreasing
            brightness = (brightness*120.0 + proposedBrightness) / 121.0; // 120
        }

        if(Microphone::getHighNormal() > highVal){
            highVal = (highVal*50.0 + Microphone::getHighNormal()) / 51.0;
        }else{
            highVal = (highVal*50.0 + Microphone::getHighNormal()) / 51.0;//25
        }
        
        
    }
    if(highVal > 1){
        highVal = 1;
    }
    
    if(proposedBrightness > brightness){
        brightness = proposedBrightness;
    }

    if(brightness > highVal){
        result = brightness + highVal * 0.1;
    }else{
        result = highVal + brightness * 0.1;
    }
    
    create_new_sparks((int)(result * 50) + 1);
    //FireV0_logo->setBrightness((float)result);
}

void FireV0::release(){
    delete(secTimer);
    delete(avgTimer);
    delete(valTimer);
    //delete(FireV0_logo);
}