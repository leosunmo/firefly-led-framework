#pragma once
#include "../Controllers/Controller.h"
#include "colorUtil.h"
#include "LEDChange.h"
#include "../../config.h"

// Currently using way too much memory here...
#define MAX_NUM_LEDS NUM_LEDS // working up to 3000 (when 1 strip)

class LEDStrip{
    /*
	• HW independent
	• Manages LEDs, count, current values, conversions
    • Calls upon controller object to output
    • Everything is normalized to 1.0 (this makes life easier)
        • Fixed vs Floating performance is negligible from my quick test4
	• HSV/RGB support
    • Adaptive framerate based on furthest LED index updated
    */

    public:
        LEDStrip(uint8_t strip);
        void apply();   // Apply the changes 
        void output();  // and output
        void clear();   // clear
        /**
         * Set RGB color using floating-point values (0.0-1.0)
         * This method performs bounds checking on the index and sanitizes RGB values.
         */
        void setRGB(int index, rgb_t rgb);
        
        /**
         * Faster RGB setting for internal use.
         * This method can be used when the rgb8_t values are already known to be valid
         * and the caller has already handled index bounds checking.
         * For example, when reusing rgb8 values returned from setHSV.
         */
        void setRGBUnprotected(int index, rgb8_t rgb);
        irgb8_t setHSV(int index, hsv_t hsv);
        void giveController(Controller *);
        double num(); //Get number of LEDs
        void setNum(uint16_t num); //set number of LEDs
    private:
        Controller * ledController;
        LEDChange* changesArray[MAX_NUM_LEDS]; // Need to update this length or make it dynamic!!
        uint8_t ledsArray[MAX_NUM_LEDS*3];
        uint16_t numLEDs = 1;
        

        uint8_t strip = 0;
};