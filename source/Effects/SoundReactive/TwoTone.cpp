#include "TwoTone.h"
#include "stdio.h"
#include <cmath>
#include <algorithm>

void TwoTone::init()
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
    
    // Initialize with default values
    hue1 = 0.0f;            // Red (shown when quiet)
    hue2 = 0.66f;           // Blue (mixed in as audio increases)
    pulsePeriod = 1.0f;     // Not used in this simpler implementation
    pulseReactivity = 5.0f; // Higher values mean more color change for the same audio level
    pulsePosition = 0.0f;   // Start at first hue
}

void TwoTone::run()
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

    printf("MicVal: %f\tMicAdd: %f\tsmoothingMicVal: %f\n", micVal, micAdd, smoothingMicVal);
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

    // Use audio level to drive the pulse position directly
    // This maps audio volume to the color transition between the two hues
    
    // Scale the mic value by the reactivity
    float scaledMicValue = micVal * pulseReactivity;
    
    // Constrain to a reasonable range (0.0-1.0)
    scaledMicValue = std::min(1.0f, std::max(0.0f, scaledMicValue));
    
    // Fast attack, slower decay for more natural audio response
    float attackRate = 0.4f;  // Lower value = faster attack
    float decayRate = 0.95f;  // Higher value = slower decay
    
    // Quick rise on audio peaks, slower falloff when quiet
    if (scaledMicValue > pulsePosition) {
        // Fast attack when audio increases
        pulsePosition = pulsePosition * attackRate + scaledMicValue * (1.0f - attackRate);
    } else {
        // Slower decay when audio decreases
        pulsePosition = pulsePosition * decayRate;
    }
    
    // In very quiet situations, ensure it returns to default position
    // This means it will show hue1 when quiet, and move toward hue2 as audio increases
    if (micVal < 0.01f && pulsePosition < 0.01f) {
        pulsePosition = 0.0f;
    }

    // Scale the brightness increase based on audio level - just like FullBar does
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
    
    // Simple linear interpolation between the two hues based on pulse position
    // pulsePosition is directly driven by audio level, so this gives us audio-reactive colors
    
    // Handle shortest path for hue interpolation
    float hueDiff = hue2 - hue1;
    if (hueDiff > 0.5f) hueDiff -= 1.0f;
    if (hueDiff < -0.5f) hueDiff += 1.0f;
    
    // Linear interpolation between the two hues
    float currentHue = hue1 + hueDiff * pulsePosition;
    
    // Normalize to [0,1] range
    if (currentHue < 0.0f) currentHue += 1.0f;
    if (currentHue >= 1.0f) currentHue -= 1.0f;
    
    // Create the HSV color
    hsv_t currentColor = {currentHue, 1.0f, 1.0f};
    
    // Iterate through all the pixels and set their values.
    for (SoundPixel *pixel : *pixels)
    {
        pixel->brightness = brightness;
        pixel->micVal = micVal;
        
        // Set hue directly from the pulse calculation
        pixel->hue = currentColor.h; // hue is already in [0,1] range
        pixel->saturation = currentColor.s;
        // brightness is handled separately
    }
}

void TwoTone::resetSecTimer()
{
    secTimer->reset();
}

void TwoTone::setHues(float hue1, float hue2)
{
    setHue1(hue1);
    setHue2(hue2);
}

void TwoTone::setHue1(float value)
{
    // Normalize hue value to [0,1] range
    hue1 = value < 0 ? fmod(value, 1.0f) + 1.0f : fmod(value, 1.0f);
}

void TwoTone::setHue2(float value)
{
    // Normalize hue value to [0,1] range
    hue2 = value < 0 ? fmod(value, 1.0f) + 1.0f : fmod(value, 1.0f);
}

void TwoTone::setPeriod(float seconds)
{
    // Ensure a minimum period to prevent division by zero
    pulsePeriod = std::max(0.1f, seconds);
}

void TwoTone::setReactivity(float value)
{
    // Clamp reactivity to [0,10] range (allowing much higher values for stronger response)
    pulseReactivity = std::min(10.0f, std::max(0.0f, value));
}

TwoTone::~TwoTone()
{
    delete (secTimer);
    delete (hueTimer);
    delete (brightnessTimer);
    delete (pixels);
}
