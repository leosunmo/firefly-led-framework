#include "ShakeelFlashBall.h"
#include <stdio.h>
#include <math.h>
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"

void ShakeelFlashBall::init()
{
    printf("Initialized ShakeelFlashBall\n");
    const int maxBaseSpeed = 300;
    const int minBaseSpeed = 50;
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
                        
            // Map event.value (0-100) to range minBaseSpeed-maxBaseSpeed
            if(event.value >= 0 && event.value <= 100) {
                baseSpeed = minBaseSpeed + ((maxBaseSpeed - minBaseSpeed) * event.value) / 100.0;
                printf("Mapped encoder value %d to speed %.1f (range %d-%d)\n", 
                       event.value, baseSpeed, minBaseSpeed, maxBaseSpeed);
            }
    } else if (event.type == FireFly::InputType::VALUE_CHANGE) {
            // Map event.value (0-100) to range minBaseSpeed-maxBaseSpeed
            if(event.value >= 0 && event.value <= 100) {
                baseSpeed = minBaseSpeed + ((maxBaseSpeed - minBaseSpeed) * event.value) / 100.0;
                printf("Mapped UART value %d to speed %.1f (range %d-%d)\n", 
                        event.value, baseSpeed, minBaseSpeed, maxBaseSpeed);
            }
    }
    });
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
    double lowMicVal = pow(Microphone::getLowNormal(), 2);
    double midMicVal = pow(Microphone::getMidNormal(), 2);
    double highMicVal = pow(Microphone::getHighNormal(), 2);
    bar->micVal = lowMicVal;
    bar->baseBrightness = baseBrightness;
    bar->run();

    // If the punch button is pressed, increase the brightness and set the hue to a bright color
    if (punch)
    {
        // Let the punch run for a half second
        if (punchTimer->timerMs() < 500)
        {
            bar->brightness += 0.9; // Boost brightness
            bar->smoothingMicVal = 20; // Lower smoothing for a more responsive effect
            ballTriggerThreshold -= 0.2; // Boost ball trigger threshold
        }
        else
        {
            punch = false; // Reset the punch state

            // Reset the secTimer so hueAdd doesn't flash forward.
            bar->resetSecTimer();
            bar->brightness = baseBrightness; // Reset brightness to base level
            bar->smoothingMicVal = 50; // Reset smoothing to a lower value
            ballTriggerThreshold = 0.5; // Reset ball trigger threshold
        }
    }

    // Add BounceBalls
    static bool ball_ready = true;
    if (highMicVal > ballTriggerThreshold)
    {
        if (ball_ready)
        {
            // Make new BOunce Ball
            BounceBall *ball = new BounceBall();
            ball->init();

            // Set speed based on highMicVal
            double speed = 50.0 * highMicVal / 0.8 + baseSpeed * (highMicVal / 0.8); // Scale lowMicVal and add baseSpeed as a booster
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