#include "ShakeelFlashBall.h"
#include <stdio.h>
#include <math.h>
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"



void ShakeelFlashBall::init(){
    const int maxBaseSpeed = 100;
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

    bool ballDirection = true;

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

    // Register encoder callback
    ShakeelFlashBall::effectEncoder->setCallback([this](int count)
{
        printf("Encoder count: %d\n", count);
        if (count > 0)
        {
            if (baseSpeed < maxBaseSpeed)
            {
                baseSpeed += 10;
            }
        }
        else
        {
            baseSpeed -= 10;
            if (baseSpeed < 0)
            {
                baseSpeed = 0;
            }
        }
});

    // Register button callback as "punch" button
    ShakeelFlashBall::effectButton->setCallback([this]()
{
        // Reset the punch timer
        punchTimer->reset();
        ShakeelFlashBall::punch = true;
});

    secTimer = new Timing();
    avgTimer = new Timing();
    valTimer = new Timing();
    punchTimer = new Timing();
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

    // If the punch button is pressed, increase the brightness and set the hue to a bright color
    if (punch)
    {
        // Let the punch run for a half second
        if (punchTimer->timerMs() < 500)
        {
            brightness += 10.0; // Boost brightness
        }
        else
        {
            punch = false; // Reset the punch state

            // Reset the secTimer so hueAdd doesn't flash forward.
            secTimer->reset();
        }
    }
   
    for(SoundPixel *pixel : *pixels){
        pixel->brightness = brightness;
        pixel->micVal = micVal;
        if (punch)
        {
            pixel->hue = 1.0; // Set to a bright hue (e.g., orange)
        }
        else
        {
            pixel->hueAdd = hueAdd * seconds * 100.0; // Use the calculated hue
        }
    }

    // Add BounceBalls
    static bool ball_ready = true;
    if (micVal > 0.5)
    {
        if (ball_ready) {
            // Make new BOunce Ball
            BounceBall * ball = new BounceBall();
            ball->init();

            // Set speed based on micVal
            double speed = 50.0 * micVal / 0.8 + baseSpeed * (micVal / 0.8); // Scale micVal and add baseSpeed as a booster
            printf("BaseSpeed: %f\tSpeed: %f\n", baseSpeed, speed);

            // Vary the ball direction each time
            printf("Ball direction: %d\n", ballDirection);
            int pos = ballDirection ? 0 : static_cast<int>(LEDs::strip(0)->num())-1;
            ball->reset(pos, ballDirection ? 1 : 0, 1, (rand()%100)/100.0, 0.01, speed);
            ballDirection = !ballDirection; // Toggle direction for the next ball
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
    effectEncoder->clearCallbacks();
    effectButton->clearCallbacks();
    delete(secTimer);
    delete(avgTimer);
    delete(pixels);
}