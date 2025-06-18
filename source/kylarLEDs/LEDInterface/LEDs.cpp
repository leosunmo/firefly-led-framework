#include "LEDs.h"
#include <stdio.h>

std::vector<LEDStrip*> *LEDs::strips;
bool ledsUseGlobalBrightness = false;
double* ledsGlobalBrightness;
LEDs::LEDs(){


}



LEDStrip *LEDs::strip(uint8_t strip){
    return strips->at(strip);
}

void LEDs::init(uint8_t numInterfaces){
    ColorUtil::generateRemapLUT();
    strips = new std::vector<LEDStrip*>();
    printf("size of LEDStrip = %d\n", sizeof(LEDStrip));
    for(int i = 0; i < numInterfaces; i++){
        LEDStrip *leds = new LEDStrip(i);
        leds->setNum(1); // Setting a default number of LEDs...
        LEDs::strips->push_back(leds);
    }
}

// Sets hsv for all strips
void LEDs::setHSV(int i, hsv_t color){
    irgb8_t irgb;
    int first = 1;
    if(ledsUseGlobalBrightness){
        color.v *= (*ledsGlobalBrightness);
    }
    // The reason the stuff going on here is weird, is to make it faster
    // Using the result of the first setHSV to make the next faster
    for(LEDStrip *strip : *strips){
        if(first){
            irgb = strip->setHSV(i, color);
            first = 0;
        }else{
            //irgb = strip->setHSV(i, color);
            strip->setRGBUnprotected(irgb.i, irgb.rgb);  // This skips the HSV conversion, which saves time
        }
        
    }
}

// Sets rgb for all strips using 8-bit RGB values (range 0-255)
void LEDs::setRGB(int i, rgb8_t color){
    // Using setRGBUnprotected is still valid here since the index will be handled
    // by each strip and we're passing 8-bit RGB values that don't need conversion
    for(LEDStrip *strip : *strips){
        strip->setRGBUnprotected(i, color);  // This skips HSV conversion and additional bounds checking
    }
}

// Sets rgb for all strips using floating-point RGB values (range 0.0-1.0)
void LEDs::setRGB(int i, rgb_t color){
    // Handle global brightness if enabled
    if(ledsUseGlobalBrightness){
        color.r *= (*ledsGlobalBrightness);
        color.g *= (*ledsGlobalBrightness);
        color.b *= (*ledsGlobalBrightness);
    }
    
    // Apply to all strips using the full protected method
    for(LEDStrip *strip : *strips){
        strip->setRGB(i, color);
    }
}

void LEDs::setNum(uint16_t num){
    for(LEDStrip *strip : *strips){
        strip->setNum(num);
    }
}

void LEDs::output(){
    for(LEDStrip *strip : *strips){
        strip->output();
    }
}

void LEDs::clear(){
    for(LEDStrip *strip : *strips){
        strip->clear();
    }

}

void LEDs::apply(){
    for(LEDStrip *strip : *strips){
        strip->apply();
    }
}

void LEDs::giveController(Controller * controller){
    for(LEDStrip *strip : *strips){
        strip->giveController(controller);
    }
}

void LEDs::useGlobalBrightnessControl(bool enable, double* globalBrightness){
    ledsUseGlobalBrightness = enable;
    ledsGlobalBrightness = globalBrightness;
}