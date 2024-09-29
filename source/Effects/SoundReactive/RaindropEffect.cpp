#include "RaindropEffect.h"
#include "stdio.h"
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"

void RaindropEffect::init(){
    printf("Initialized raindropEffect");
    pixels = new std::vector<Pixel>();
}

void RaindropEffect::move_pixel(int position) {
    // Make sure it's not the last pixel, then move it
    if (position + 1 < pixels->size()) {
        pixels->at(position+1).wateryness += pixels->at(position).wateryness;
        
        // Random chance for the pixel to no longer be 'moving'
        if (rand() % 10000 < 30) {
            pixels->at(position+1).moving = false;
        } else {
            pixels->at(position+1).moving = true;
        }
    }
    // clear the pixel after the drop has moved
    pixels->at(position).wateryness = 0;
    pixels->at(position).moving = false;
}

void RaindropEffect::run(){
    if (myTiming->everyMs(1)) {
        micValue = pow(Microphone::getLowNormal(),2)*1000;
        if (micValue <= previousMicValue) {
            micValue = MIN((double)(micValue*1.0+previousMicValue*99.0)/100.0, 1000);
        }
        previousMicValue = micValue;

        if (myTiming->everyMs(2000)) {
            printf("micValue: %f", micValue);
        }
        
        // Add water to a random pixel sometimes, more often based on micValue
        if (rand() % 10000 < 5 + micValue) {
            pixels->at(rand() % pixels->size()).wateryness += rand() % 2 + 1;
        }
        // start moving a random pixel sometimes
        for (int i=0; i<pixels->size(); i++){
            if (rand() % 10000 < pixels->at(i).wateryness + micValue*0) {
                pixels->at(i).moving = true;
            }
            // If the current pixel is moving, move it sometimes, 
            // more likely with higher wateryness and micValue.
            if (pixels->at(i).moving && rand() % 10000 < (5 + pixels->at(i).wateryness*2 + micValue*0)) {
                move_pixel(i);
            }
            hsv_t color = {0.55, 1, 1};
            color.v = pixels->at(i).wateryness/100.0;
            LEDs::setHSV(pixels->at(i).led, color);
            // draw up to 6 tailing lights if there is room and brightness is enough
            for (int j=1;j<7;j++) {
                if (i >= j && pixels->at(i).moving) {
                    color.v *= 0.5;
                    LEDs::setHSV(pixels->at(i-j).led, color);
                }
            }
        }
    }
}

RaindropEffect::~RaindropEffect(){
    delete(pixels);
}
