#pragma once
#include "../Effect.h"
#include "../../kylarLEDs/LEDInterface/colorUtil.h"

class SoundPixel : public Effect{

    public:
        using Effect::Effect;
        ~SoundPixel();
        void run();
        void init();
        
        // Set mode to use RGB color (direct interpolation) instead of HSV
        void useRGBMode(bool useRGB) { rgbMode = useRGB; }
        bool isRGBMode() const { return rgbMode; }
        
        // Set RGB color directly
        void setRGB(float r, float g, float b) {
            red = r;
            green = g;
            blue = b;
            // Automatically switch to RGB mode
            rgbMode = true;
        }
        
        uint16_t i = 0;
        uint8_t strip = 0;
        float hue = 0;
        float micVal = 0;
        float hueAdd = 0;
        float saturation = 1;
        float brightness = 0;
        
        // RGB components for direct RGB mode
        float red = 0;
        float green = 0;
        float blue = 0;
        
    private:
        // When true, use RGB colors directly instead of HSV
        bool rgbMode = false;
};