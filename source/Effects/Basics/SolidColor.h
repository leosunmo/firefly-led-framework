#pragma once
#include "../Effect.h"
#include "../../kylarLEDs/LEDInterface/colorUtil.h"
#include "../../Effects/SoundReactive/SoundPixel.h"


/**
 * SolidColor Effect
 * 
 * A simple effect that shows a single solid color at full brightness.
 * The hue can be changed dynamically through the setHue() method.
 */
class SolidColor : public Effect {
public:
    using Effect::Effect;
    void run() override;
    void init() override;
    ~SolidColor();
    
    /**
     * Set the hue value (0.0-1.0)
     * This applies inverse mapping to ensure colors appear as expected
     * @param value The desired hue (0.0-1.0) to display
     */
    void setHue(float value);
    
    /**
     * Get the current hue value
     * @return The current hue (0.0-1.0)
     */
    float getHue() const { return hue; }
    
    /**
     * Set the saturation value (0.0-1.0)
     * @param value The desired saturation (0.0-1.0)
     */
    void setSaturation(float value);
    
    /**
     * Get the current saturation value
     * @return The current saturation (0.0-1.0)
     */
    float getSaturation() const { return saturation; }
    
    /**
     * Set the brightness value (0.0-1.0)
     * @param value The desired brightness (0.0-1.0)
     */
    void setBrightness(float value);
    
    /**
     * Get the current brightness value
     * @return The current brightness (0.0-1.0)
     */
    float getBrightness() const { return brightness; }

private:
    float hue = 0.0f;            // Current hue value (0.0-1.0)
    float saturation = 1.0f;     // Current saturation value (0.0-1.0)
    float brightness = 1.0f;     // Current brightness value (0.0-1.0)
    float mappedHue = 0.0f;      // Mapped hue value for LEDStrip system

    std::vector<SoundPixel*> *pixels;

};
