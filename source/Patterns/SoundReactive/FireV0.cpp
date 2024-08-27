#include "FireV0.h"
#include <stdio.h>
#include "../../Effects/Basics/ExampleEffect.h"
#include "../../Effects/Effect.h"
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"
#include "../../kylarLEDs/Utility/Waveforms/Triangle.h"


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

    LEDs::useGlobalBrightnessControl(true, &(this->global_brightness));
}

int FireV0::calc_num_new_sparks(){
    // Currently the Run takes care of this
    return 0;
}

void FireV0::create_new_sparks(int x_sparks){
    static int left_side = 0;
    static int right_side = NUM_LEDS - 1;
    static int side = 0; // Flip the side they're created on
    static int spark_index = 0; // Increment this so we wrap around sparks array
    static double spark_hue = 1.17;

    int spark_pos;
    int spark_dir;
    double brightness = 0.2;
    int steps = x_sparks;
    double speed = x_sparks / 10.0;
    for(int i = 0; i < x_sparks/2; i++){

        /*
         * Try to grab a spark
         */
        Spark* spark = sparks[spark_index];
        if(spark->num_steps + spark->bonus_steps > 10){
            // This one still has steps left
            //i--;
            if(++spark_index >= num_sparks){
                spark_index = 0;
            }
            continue;
        }

        /*
         * Calculate new spark's properties
         */
        spark_pos = side ? left_side : right_side;
        spark_dir = side ? 1 : -1;
        steps = std::min(2*i, NUM_LEDS/2 - 2);
        brightness = (double)x_sparks/10.0;//(2*i)/(double)x_sparks;
        speed = x_sparks/10.0 + i*0.4; 

        /**
         * Reset a spark.
         */
        spark->reset(spark_pos,
                     spark_dir,
                     brightness,
                     steps,
                     spark_hue,
                     -0.01 + (i % 2)*0.003 /*hue shift */,
                     speed);
        
        /*
         * Fulfill our continuation goals.
         */
        side = !side;
        spark_hue += 0.01;
        if(spark_hue > 1.22){
            spark_hue = 1.17;
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
    avgLoops = avgTimer->takeMsEvery(1);

    if(useSound == false){
        /**
         * 
         * Add a few triangle waves of different periods here
         * To get a nicer brightness and differing number of sparks
         * For the fire effect :)
         */
        global_brightness = 0.8;
        create_new_sparks(20);
        return;
    }
    
    // Color movement
    //printf("micTimer = %d ... %f\n", micTimer->timerMs(), micTimer->takeSeconds());

    // printf("avgTimer = %d ... %d\n", avgTimer->timerMs());//, avgTimer->takeMsEvery(25));
    


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
    if(result > 0.8){
        // Extend sparks on loud sounds
        for(int i = 0; i < num_sparks; i++){
            Spark* spark = sparks[i];
            if(spark->num_steps > 0 && spark->bonused == 0){
                spark->bonus_steps = 5;
                spark->bonused = 1;
            }
        }
    }
    global_brightness = 0.35*result+0.65;
    create_new_sparks((int)(result*5)*(result * 5) + 2);
    
    //FireV0_logo->setBrightness((float)result);
}

void FireV0::release(){
    delete(secTimer);
    delete(avgTimer);
    delete(valTimer);
    for(int i = 0; i < num_sparks; i++){
        delete(sparks[i]);
    }
    //delete(FireV0_logo);
}