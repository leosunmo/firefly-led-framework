#include "SolidColor.h"
#include "stdio.h"

void SolidColor::init() {
    // Nothing special to initialize
    mappedHue = hue; // Initialize mapped hue

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
}

void SolidColor::run() {
    // Set all LEDs to the same color
    hsv_t color = {mappedHue, saturation, brightness};
    // Iterate through all the pixels and set their values.
    for (SoundPixel *pixel : *pixels)
    {
        pixel->brightness = brightness;
        pixel->hue = mappedHue;
        pixel->saturation = saturation;
    }
}

void SolidColor::setHue(float value) {
    // Clamp hue to 0.0-1.0 range
    if (value > 1.0f) value = 1.0f;
    if (value < 0.0f) value = 0.0f;
    
    hue = value;
    
    // Apply any necessary mapping for the LEDStrip system
    // Currently, we use the direct value, but this allows for future
    // color correction if needed
    mappedHue = hue;
    
    // Print debug info
    printf("SolidColor - Setting hue: %f\n", hue);
}

void SolidColor::setSaturation(float value) {
    // Clamp saturation to 0.0-1.0 range
    if (value > 1.0f) value = 1.0f;
    if (value < 0.0f) value = 0.0f;
    
    saturation = value;
    printf("SolidColor - Setting saturation: %f\n", saturation);
}

void SolidColor::setBrightness(float value) {
    // Clamp brightness to 0.0-1.0 range
    if (value > 1.0f) value = 1.0f;
    if (value < 0.0f) value = 0.0f;
    
    brightness = value;
    printf("SolidColor - Setting brightness: %f\n", brightness);
}

SolidColor::~SolidColor() {
    delete (pixels);
    pixels = nullptr; // Avoid dangling pointer
}
