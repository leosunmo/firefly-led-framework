#include "ShakeelFlash.h"
#include <stdio.h>
#include <math.h>
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"

void ShakeelFlash::init()
{
    printf("Initialized ShakeelFlash\n");
    bar = new FullBar();
    bar->init();

    // Register with InputManager instead of directly with encoder
    auto& inputManager = FireFly::InputManager::getInstance();
    
    // Subscribe to effect encoder events
    inputManager.subscribe(FireFly::InputSource::HW_EFFECT_ENCODER, [this](const FireFly::InputEvent& event) {
        printf("Encoder event: %d\n", event.value);
        
        // Determine if the encoder went up or down based on previous value
        static int prevValue = 0;
        int change = event.value - prevValue;
        prevValue = event.value;
        
        if (change > 0)
        {
            if (baseBrightness < 0.95)
            {
                baseBrightness += 0.05;
            }
        }
        else if (change < 0)
        {
            baseBrightness -= 0.05;
            if (baseBrightness < 0.05)
            {
                baseBrightness = 0.05;
            }
        }
        printf("Base Brightness: %f\n", baseBrightness);
    });
    
    // Also subscribe to UART custom param for the same functionality
    inputManager.subscribe(FireFly::InputSource::UART_CUSTOM_PARAM_1, [this](const FireFly::InputEvent& event) {
        printf("UART brightness param: %d\n", event.value);
        
        // Value from UART should be brightness value (0-100)
        double brightness = event.value / 100.0;
        if (brightness >= 0.05 && brightness <= 0.95) {
            baseBrightness = brightness;
            printf("Base Brightness (from UART): %f\n", baseBrightness);
        }
    });
}

void ShakeelFlash::run()
{
    double micVal = pow(Microphone::getLowNormal(), 2);
    bar->micVal = micVal;
    bar->baseBrightness = baseBrightness;
    bar->run();
}

void ShakeelFlash::release()
{
    // Unsubscribe from input events instead of clearing callbacks directly
    auto& inputManager = FireFly::InputManager::getInstance();
    inputManager.unsubscribeAll(FireFly::InputSource::HW_EFFECT_ENCODER);
    inputManager.unsubscribeAll(FireFly::InputSource::UART_CUSTOM_PARAM_1);
    
    delete(bar);
}