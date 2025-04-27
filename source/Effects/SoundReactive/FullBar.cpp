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
    double micAdd = micVal / 50; // trying to even out timing

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

    // Set proposed brightness based on mic value
    // The brightness is set to a minimum of 0.1 and a maximum of 0.9.
    double proposedBrightness = 0.1 + (0.9 * micVal);

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

FullBar::~FullBar()
{
    delete (secTimer);
    delete (hueTimer);
    delete (brightnessTimer);
    delete (pixels);
}