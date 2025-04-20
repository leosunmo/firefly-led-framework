#include "Bounce.h"
#include <stdio.h>
#include <math.h>
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"

int Bounce::maxAllowedThickness = 10; // Define static member with an initial value

void Bounce::init()
{
    printf("Initialized Bounce with config: SolidBar=%s, SplitBar=%s, HighLowInput=%s, MaxThickness=%d\n",
           config.solidBar ? "true" : "false",
           config.splitBar ? "true" : "false",
           config.highLowInput ? "true" : "false",
           config.maxThickness);
    int stripLen = LEDs::strip(0)->num();

    // Register encoder callback
    Bounce::effectEncoder->setCallback([this](int count)
                                       {
                                        printf("Encoder count: %d\n", count);
        if (count > 0)
        {
            if (config.maxThickness < Bounce::maxAllowedThickness)
            {
                config.maxThickness++;
            }
        }
        else
        {
            if (config.maxThickness > 0)
            {
                config.maxThickness--;
            }
        } 

            printf("Max Thickness: %d\n", config.maxThickness); });

    // Register button callback
    Bounce::effectButton->setCallback([this]()
                                      { GPIOInterruptHandler::printCallbacks(); });

    secTimer = new Timing();
    avgTimer = new Timing();
    valTimer = new Timing();
}

void Bounce::run()
{
    double micVal;
    if (config.highLowInput)
    {
        double lowVal = pow(Microphone::getLowNormal(), 2);
        double highVal = pow(Microphone::getHighNormal(), 2);
        micVal = (lowVal < 0.1) ? (0.5 * lowVal + 0.5 * highVal) : lowVal;
    }
    else
    {
        micVal = pow(Microphone::getLowNormal(), 2);
    }

    // double micAdd = micVal / 50; // trying to even out timing

    // int avgLoops = avgTimer->takeMsEvery(1);
    // for (int i = 0; i < avgLoops; i++) {
    //     brightness = (brightness * 100.0 + proposedBrightness) / 101.0;
    // }

    static double brightness = 0;
    double proposedBrightness = 0.1 + (0.9 * micVal);

    int fillLength = static_cast<int>(micVal * LEDs::strip(0)->num());

    double smoothingFactor = 0.9;
    brightness = brightness * (1.0 - smoothingFactor) + proposedBrightness * smoothingFactor;

    // Calculate thickness based on micVal
    int thickness = 1 + static_cast<int>(micVal * (config.maxThickness - 1));

    static double velocity1 = 0, velocity2 = 0;
    static double position1 = 0, position2 = 0;
    static double gravity = -0.003;
    static double bounceFactor = -0.7;

    int stripLen = LEDs::strip(0)->num();
    int halfStripLen = stripLen / 2;

    static bool ball_ready = true;
    if (micVal > 0.5)
    {
        if (ball_ready) {
            // Make new BOunce Ball
            BounceBall * ball = new BounceBall();
            ball->init();
            ball->reset(0, 1, 1, (rand()%100)/100.0, 0.01, (rand()%100)/33.0);
            Effect::engine->queueApply(ball);
        }
        ball_ready = false;
        
        // if (fillLength < position1) {
        //     position1 = fillLength;
        //     velocity1 = 0;
        //     position2 = fillLength;
        //     velocity2 = 0;
        // }
    }
    else
    {
        ball_ready = true;
        // velocity1 += gravity;
        // position1 += velocity1;

        // if (position1 < 0)
        // {
        //     position1 = 0;
        //     velocity1 *= bounceFactor;
        // }

        // if (config.splitBar)
        // {
        //     velocity2 += gravity;
        //     position2 += velocity2;

        //     if (position2 < 0)
        //     {
        //         position2 = 0;
        //         velocity2 *= bounceFactor;
        //     }
        // }
    }

    return;
    
    for (int i = 0; i < stripLen; i++)
    {
        if (config.splitBar)
        {
            // Check if the pixel is within the thickness range of position1 or position2
            if ((i >= halfStripLen - static_cast<int>(position1) - thickness / 2 && i <= halfStripLen - static_cast<int>(position1) + thickness / 2) ||
                (i >= halfStripLen + static_cast<int>(position2) - thickness / 2 && i <= halfStripLen + static_cast<int>(position2) + thickness / 2))
            {
                (*pixels)[i]->brightness = brightness;
                (*pixels)[i]->hue = 0.0; // Set to initial color when lit
            }
            else
            {
                // Fade out and change color
                (*pixels)[i]->brightness *= 0.95; // Slowly fade out
                if ((*pixels)[i]->hue < 1.0)
                {
                    (*pixels)[i]->hue += 0.01; // Slowly change color
                }
                else
                {
                    (*pixels)[i]->hue = 1.0; // Cap the hue at 1.0
                }
            }
        }
        else
        {
            // Check if the pixel is within the thickness range of position1
            if (i >= static_cast<int>(position1) - thickness / 2 && i <= static_cast<int>(position1) + thickness / 2)
            {
                (*pixels)[i]->brightness = brightness;
                (*pixels)[i]->hue = 0.0; // Set to initial color when lit
            }
            else
            {
                // Fade out and change color
                (*pixels)[i]->brightness *= 0.95; // Slowly fade out
                if ((*pixels)[i]->hue < 1.0)
                {
                    (*pixels)[i]->hue += 0.01; // Slowly change color
                }
                else
                {
                    (*pixels)[i]->hue = 1.0; // Cap the hue at 1.0
                }
            }
        }
        (*pixels)[i]->run();
    }
}

void Bounce::release()
{
    effectEncoder->clearCallbacks();
    effectButton->clearCallbacks();

    delete (secTimer);
    delete (avgTimer);
    delete (valTimer);
    delete pixels;
}
