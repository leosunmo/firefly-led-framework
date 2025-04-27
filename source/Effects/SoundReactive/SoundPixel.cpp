#include "SoundPixel.h"
#include "stdio.h"
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"

void SoundPixel::init()
{
    printf("Initialized SoundPixel\n");
}

void SoundPixel::run()
{
    // If hueAdd is really low, ignore it
    if (hueAdd > 0.00001)
    {
        hue += hueAdd; // 0.01- 0.001
    }

    // hsv_t
    // float h;         // Color: 0 to 1.0 : 0 is RED, 0.33 is GREEN, 0.66 BLUE
    // float s;        // Whiteness: 0 to 1.0 : 0 is WHITE, 1 is COLOR
    // float v;       // Brightness: 0 to 1.0 : 0 is OFF, 1 is ON

    // Output
    hsv_t color = {hue, saturation, brightness};
    LEDs::strip(strip)->setHSV(i, color);
}

SoundPixel::~SoundPixel()
{
    done = 1;
}
