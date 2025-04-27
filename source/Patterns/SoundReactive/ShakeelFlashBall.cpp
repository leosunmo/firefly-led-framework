#include "ShakeelFlashBall.h"
#include <stdio.h>
#include <math.h>
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"



void ShakeelFlashBall::init(){
    printf("Initialized ShakeelFlashBall\n");
    const int maxBaseSpeed = 100;
    bar = new FullBar();
    bar->init();
    bool ballDirection = true;

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

punchTimer = new Timing();

    // Register button callback as "punch" button
    ShakeelFlashBall::effectButton->setCallback([this]()
{
        // Reset the punch timer
        punchTimer->reset();
        ShakeelFlashBall::punch = true;
});

}

void ShakeelFlashBall::run(){
    double micVal = pow(Microphone::getLowNormal(),2) ;
    bar->micVal = micVal;
    bar->baseBrightness = baseBrightness;
    bar->run();

    // If the punch button is pressed, increase the brightness and set the hue to a bright color
    // if (punch)
    // {
    //     // Let the punch run for a half second
    //     if (punchTimer->timerMs() < 500)
    //     {
    //         brightness += 10.0; // Boost brightness
    //     }
    //     else
    //     {
    //         punch = false; // Reset the punch state

    //         // Reset the secTimer so hueAdd doesn't flash forward.
    //         secTimer->reset();
    //     }
    // }

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
    delete(punchTimer);
    delete(bar);
}