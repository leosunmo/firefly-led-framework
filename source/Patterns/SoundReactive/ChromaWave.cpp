#include "ChromaWave.h"
#include <math.h>

ChromaWave::ChromaWave()
{
    this->twoToneEffect = new TwoTone();
}

void ChromaWave::init()
{
    // Initialize the TwoTone effect
    twoToneEffect->init();
    
    // Apply the effect to the engine
    Effect::engine->apply(twoToneEffect);
    
    // Initialize with default hues
    twoToneEffect->setHues(hue, hue2);
}

void ChromaWave::run()
{
    // Get audio levels from the microphone
    double lowFreq = Microphone::getLowNormal();
    
    // Square the value to make it more reactive to peaks
    double micVal = lowFreq * lowFreq;
    
    // Feed the processed microphone value to the effect
    twoToneEffect->micVal = micVal;
}

void ChromaWave::setHue(double h)
{
    // First hue comes from standard hue parameter
    hue = h;
    if (twoToneEffect)
    {
        twoToneEffect->setHue1(hue);
    }
}

void ChromaWave::setSecondHue(double h)
{
    // Second hue is a custom parameter
    hue2 = h;
    if (twoToneEffect)
    {
        twoToneEffect->setHue2(hue2);
    }
}

void ChromaWave::setBrightness(float b)
{
    if (twoToneEffect)
    {
        twoToneEffect->baseBrightness = b;
    }
}

void ChromaWave::setSpeed(float speed)
{
    // Map the speed parameter to audio reactivity
    // Higher speed = more color change for the same audio level
    if (twoToneEffect)
    {
        // Map speed (typically 0-1) to reactivity (1.0 - 10.0)
        float reactivity = 1.0f + (speed * 9.0f);
        twoToneEffect->setReactivity(reactivity);
    }
}

void ChromaWave::setPeriod(float seconds)
{
    if (twoToneEffect)
    {
        twoToneEffect->setPeriod(seconds);
    }
}

void ChromaWave::setReactivity(float value)
{
    if (twoToneEffect)
    {
        twoToneEffect->setReactivity(value);
    }
}

const char* ChromaWave::getName()
{
    return "ChromaWave";
}

void ChromaWave::release()
{
    // Clean up is handled by the TwoTone effect's destructor
    // Nothing special to clean up here
}
