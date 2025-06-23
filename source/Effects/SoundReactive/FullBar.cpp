#include "FullBar.h"
#include "stdio.h"
#include <cmath>

void FullBar::init()
{
    pixels = new std::vector<SoundPixel *>();
    int stripLen = LEDs::strip(0)->num();
    for (int i = 0; i < stripLen; i++)
    {
        SoundPixel *eff = new SoundPixel();
        eff->i = i;
        eff->hue = 0.0;
        eff->strip = 0;
        Effect::engine->apply(eff);
        pixels->push_back(eff);
    }
    secTimer = new Timing();
    hueTimer = new Timing();
    brightnessTimer = new Timing();
}

void FullBar::run()
{
    double seconds = secTimer->takeSeconds();
    double micAdd;
    if (smoothingMicVal <= 0.0)
    {
        micAdd = micVal; // If smoothingMicVal is 0, use micVal directly
    } else
    {
        micAdd = micVal / smoothingMicVal; // trying to even out timing
    }

    int avgLoops = 0;
    avgLoops = hueTimer->takeMsEvery(1);
    for (int i = 0; i < avgLoops; i++)
    {
        hueAdd = (hueAdd * 100.0 + micAdd) / 101.0;
    }

    if (micAdd > hueAdd)
    {
        hueAdd = micAdd;
    }

    // Scale the temporary brightness increase relative to baseBrightness
    double tempBrightnessIncrease = micVal * (1.0 - baseBrightness) * baseBrightness;

    // Set proposed brightness as the sum of baseBrightness and the scaled temporary increase
    double proposedBrightness = baseBrightness + tempBrightnessIncrease;

    avgLoops = brightnessTimer->takeMsEvery(1);
    // Iterate through the number of ms since last to smooth out the brightness.
    for (int i = 0; i < avgLoops; i++)
    {
        brightness = (brightness * 200.0 + proposedBrightness) / 201.0;
    }

    // Check if the proposed brightness is greater than the current brightness
    // and update it if necessary.
    if (proposedBrightness > brightness)
    {
        brightness = proposedBrightness;
    }

    // Iterate through all the pixels and set their values.
    for (SoundPixel *pixel : *pixels)
    {
        pixel->brightness = brightness;
        pixel->micVal = micVal;
        pixel->hueAdd = hueAdd * seconds * 100.0;
    }
}

void FullBar::resetSecTimer()
{
    secTimer->reset();
}

FullBar::~FullBar()
{
    delete (secTimer);
    delete (hueTimer);
    delete (brightnessTimer);
    delete (pixels);
}