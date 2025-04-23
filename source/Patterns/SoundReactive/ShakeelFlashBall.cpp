#include "ShakeelFlashBall.h"
#include <stdio.h>
#include <math.h>
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"



void ShakeelFlashBall::init(){
    printf("Initialized ShakeelFlashBall\n");
    pixels = new std::vector<SoundPixel*>();
    int stripLen = LEDs::strip(0)->num();
    for(int i = 0; i < stripLen; i++){
        SoundPixel * eff = new SoundPixel();
        //printf("sound pixel size = %d\n", sizeof(SoundPixel));
        eff->i = i;
        eff->hue = 0.0;
        eff->strip = 0;
        Effect::engine->apply(eff);
        pixels->push_back(eff);
    }

    // A second loop for a second strip
    // for(int i = 0; i < stripLen; i++){
    //     SoundPixel * eff = new SoundPixel();
    //     //printf("sound pixel size = %d\n", sizeof(SoundPixel));
    //     eff->i = i;
    //     eff->hue = 0.5;
    //     eff->strip = 1;
    //     Effect::engine->apply(eff);
    //     pixels->push_back(eff);
    // }

    secTimer = new Timing();
    avgTimer = new Timing();
    valTimer = new Timing();
}

void ShakeelFlashBall::run(){
    double micVal = pow(Microphone::getLowNormal(),2) ;
    static double hueAdd = 0;
    static double brightness = 0;
    double seconds = secTimer->takeSeconds();
    int avgLoops = 0;
    // Color movement
    //printf("micTimer = %d ... %f\n", micTimer->timerMs(), micTimer->takeSeconds());
    double micAdd = micVal / 50; // trying to even out timing
    // printf("avgTimer = %d ... %d\n", avgTimer->timerMs());//, avgTimer->takeMsEvery(25));
    avgLoops = avgTimer->takeMsEvery(1);
    for(int i = 0; i < avgLoops; i++){
        hueAdd = (hueAdd*100.0 + micAdd )/101.0;
    }
    
    if(micAdd > hueAdd){
        hueAdd = micAdd;
    }


    // Brightness
    double proposedBrightness = 0.1+(0.9*micVal);
    //printf("valTimer = %d ... %d\n", valTimer->timerMs(), valTimer->takeMsEvery(100));
    avgLoops = valTimer->takeMsEvery(1);
    for(int i = 0; i < avgLoops; i++){
        brightness = (brightness*200.0 + proposedBrightness)/201.0;
    }
    
    if(proposedBrightness > brightness){
        brightness = proposedBrightness;
    }
    
    for(SoundPixel *pixel : *pixels){
        pixel->brightness = brightness;
        pixel->micVal = micVal;
        pixel->hueAdd = hueAdd * seconds*100.0;
    }


    // Add BounceBalls
    static bool ball_ready = true;
    if (micVal > 0.4)
    {
        if (ball_ready) {
            // Make new BOunce Ball
            BounceBall * ball = new BounceBall();
            ball->init();

            // Set speed based on micVal
            double speed = 80.0 * micVal / 0.8; // Scale micVal so that 0.8 corresponds to a speed of 50
            ball->reset(0, 1, 1, (rand()%100)/100.0, 0.01, speed);
            Effect::engine->queueApply(ball);
        }
        ball_ready = false;
    }
    else
    {
        ball_ready = true;
    }
}

void ShakeelFlashBall::release(){

    delete(secTimer);
    delete(avgTimer);
    delete(pixels);
}