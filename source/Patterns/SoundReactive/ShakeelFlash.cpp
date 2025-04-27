#include "ShakeelFlash.h"
#include <stdio.h>
#include <math.h>
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"

void ShakeelFlash::init()
{
    printf("Initialized ShakeelFlash\n");
    bar = new FullBar();
    bar->init();

    ShakeelFlash::effectEncoder->setCallback([this](int count)
    {
            printf("Encoder count: %d\n", count);
            if (count > 0)
            {
                if (baseBrightness < 0.95)
                {
                    baseBrightness += 0.05;
                }
            }
            else
            {
                baseBrightness -= 0.05;
                if (baseBrightness < 0.05)
                {
                    baseBrightness = 0.05;
                }
            }
            printf("Base Brightness: %f\n", baseBrightness);
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
    effectEncoder->clearCallbacks();
    effectButton->clearCallbacks();
    delete bar;
}