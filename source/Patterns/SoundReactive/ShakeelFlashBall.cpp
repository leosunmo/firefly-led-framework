#include "ShakeelFlashBall.h"
#include <stdio.h>
#include <math.h>
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"

void ShakeelFlashBall::init()
{
    printf("Initialized ShakeelFlashBall\n");
    const int maxBaseSpeed = 100;
    bar = new FullBar();
    bar->init();
    ballDirection = true;

    // Register with InputManager instead of directly with encoder/button
    auto &inputManager = FireFly::InputManager::getInstance();

    // Subscribe to effect encoder events for speed control
    inputManager.subscribe(FireFly::InputEventType::SPEED, [this, maxBaseSpeed](const FireFly::InputEvent &event)
                           {
        printf("Encoder event: %d\n", event.value);
        
        if(event.type == FireFly::InputType::ENCODER_ROTATION) {
            printf("Base Speed encoder rotation: %d\n", event.value);
        // Determine if the encoder went up or down based on previous value
        static int prevValue = 0;
        int change = event.value - prevValue;
        prevValue = event.value;
        
        if (change > 0)
        {
            if (baseSpeed < maxBaseSpeed)
            {
                baseSpeed += 10;
            }
        }
        else if (change < 0)
        {
            baseSpeed -= 10;
            if (baseSpeed < 0)
            {
                baseSpeed = 0;
            }
        }
    } else if (event.type == FireFly::InputType::VALUE_CHANGE) {
        printf("Base Speed UART value change: %d\n", event.value);
                // Value from UART should be direct speed value (0-100)
        int speed = event.value;
        if (speed >= 0 && speed <= maxBaseSpeed) {
            baseSpeed = speed;
            printf("Base Speed (from UART): %.1f\n", baseSpeed);
        }
    }
        printf("Base Speed: %.1f\n", baseSpeed); });

    punchTimer = new Timing();

    // Subscribe to effect button events for punch effect
    inputManager.subscribe(FireFly::InputEventType::EFFECT_PUNCH, [this](const FireFly::InputEvent &event)
                           {
        if (event.value > 0) {
            // Reset the punch timer
            punchTimer->reset();
            punch = true;
        } });
}

void ShakeelFlashBall::run()
{
    double micVal = pow(Microphone::getLowNormal(), 2);
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
        if (ball_ready)
        {
            // Make new BOunce Ball
            BounceBall *ball = new BounceBall();
            ball->init();

            // Set speed based on micVal
            double speed = 50.0 * micVal / 0.8 + baseSpeed * (micVal / 0.8); // Scale micVal and add baseSpeed as a booster
            printf("BaseSpeed: %f\tSpeed: %f\n", baseSpeed, speed);

            // Vary the ball direction each time
            printf("Ball direction: %d\n", ballDirection);
            int pos = ballDirection ? 0 : static_cast<int>(LEDs::strip(0)->num()) - 1;
            ball->reset(pos, ballDirection ? 1 : 0, 1, (rand() % 100) / 100.0, 0.01, speed);
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

void ShakeelFlashBall::release()
{
    // Unsubscribe from input events instead of clearing callbacks directly
    auto &inputManager = FireFly::InputManager::getInstance();
    inputManager.unsubscribeAll(FireFly::InputEventType::SPEED);
    inputManager.unsubscribeAll(FireFly::InputEventType::EFFECT_PUNCH);

    delete (punchTimer);
    delete (bar);
}