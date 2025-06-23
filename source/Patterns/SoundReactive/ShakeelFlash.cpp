#include "ShakeelFlash.h"
#include <stdio.h>
#include <math.h>
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"

void ShakeelFlash::init()
{
    printf("Initialized ShakeelFlash\n");
    bar = new FullBar();
    bar->init();
    punchTimer = new Timing();

    // Register with InputManager instead of directly with encoder
    auto& inputManager = FireFly::InputManager::getInstance();
    


    // Subscribe to effect encoder events
    inputManager.subscribe(FireFly::InputEventType::EFFECT_PUNCH, [this](const FireFly::InputEvent& event) {
        printf("Encoder event: %d\n", event.value);
        if (event.value > 0) {
            // Reset the punch timer
            punchTimer->reset();
            punch = true;
        } });
}

void ShakeelFlash::run()
{
    double micVal = pow(Microphone::getLowNormal(), 2);
    bar->micVal = micVal;
    bar->baseBrightness = baseBrightness;
    // If the punch button is pressed, lower reactivity threshold
    if (punch)
    {
        // Let the punch run for a half second
        if (punchTimer->timerMs() < 5000)
        {
            bar->brightness += 0.9; // Boost brightness
            bar->smoothingMicVal = 20; // Reset smoothing to a lower value
        }
        else
        {
            punch = false; // Reset the punch state

            // Reset the secTimer so hueAdd doesn't flash forward.
            bar->resetSecTimer();
            bar->brightness = baseBrightness; // Reset brightness to base level
            bar->smoothingMicVal = 50; // Reset smoothing to a lower value
        }
    }


    bar->run();
}

void ShakeelFlash::release()
{
    // Unsubscribe from input events instead of clearing callbacks directly
    auto& inputManager = FireFly::InputManager::getInstance();
    inputManager.unsubscribeAll(FireFly::InputEventType::EFFECT_PUNCH);
    delete (punchTimer);
    delete(bar);
}