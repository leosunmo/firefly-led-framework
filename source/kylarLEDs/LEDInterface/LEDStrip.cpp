#include "LEDStrip.h"
#include "../Utility/ExecTimer.h"
#include "stdio.h"
#include "stdlib.h"
LEDStrip::LEDStrip(uint8_t strip){
    rgb_t off = {0, 0, 0};
    this->strip = strip;
    for(int i = 0; i < MAX_NUM_LEDS; i++){
        changesArray[i] = new LEDChange();
        // printf("size of ledchange = %d\n", sizeof(LEDChange));
        ledsArray[i*3] = 0;
        ledsArray[i*3+1] = 0;
        ledsArray[i*3+2] = 0;
        // printf("size of ledsarray = %d\n", sizeof(ledsArray));
    }
    
}


void LEDStrip::setRGB(int index, rgb_t rgb){
    // Handle index wrapping for out-of-bounds, same as setHSV
#ifdef LEDS_OFFSET
    index += LEDS_OFFSET;
#endif
    if (index >= NUM_LEDS) {
        index = index % NUM_LEDS;
    } else if (index < -NUM_LEDS) {
        index = index % NUM_LEDS;
        index += NUM_LEDS;
    } else if (index < 0) {
        index += NUM_LEDS;
    }
    
    // Sanitize RGB values to ensure they're in the [0,1] range
    rgb.r = ColorUtil::sanitizeSV(rgb.r);
    rgb.g = ColorUtil::sanitizeSV(rgb.g);
    rgb.b = ColorUtil::sanitizeSV(rgb.b);
    
    // Convert to 8-bit RGB
    rgb8_t rgb8;
    rgb8.r = static_cast<uint8_t>(rgb.r * 255);
    rgb8.g = static_cast<uint8_t>(rgb.g * 255);
    rgb8.b = static_cast<uint8_t>(rgb.b * 255);
    
    // Apply to the changes array
    changesArray[index]->combine(rgb8);
}



irgb8_t LEDStrip::setHSV(int index, hsv_t hsv){
#ifdef LEDS_OFFSET
    index += LEDS_OFFSET;
#endif
    if (index >= NUM_LEDS) {
        index = index % NUM_LEDS;
    } else if (index < -NUM_LEDS) {
        index = index % NUM_LEDS;
        index += NUM_LEDS;
    } else if (index < 0) {
        index += NUM_LEDS;
    }
    //ExecTimer *timer = new ExecTimer();
    //timer->start("setHSV");
    
    hsv.h += ledController->getHue();
    //timer->add("ledController->getHue()");

    hsv.h = ColorUtil::sanitizeH(hsv.h);
    hsv.s = ColorUtil::sanitizeSV(hsv.s);
    hsv.v = ColorUtil::sanitizeSV(hsv.v);
    //timer->add("::sanitizeHSV()");

    hsv.h = ColorUtil::remapHueLUT[(int)(hsv.h/REMAP_LUT_RES)];//ColorUtil::remapHue(hsv.h);
    //timer->add("remapHue(hsv.h);");

    hsv16_t hsv16 = {static_cast<uint16_t>(hsv.h * HSV_HUE_MAX),
                     static_cast<uint8_t>(hsv.s * HSV_SAT_MAX),
                     static_cast<uint8_t>(hsv.v * HSV_VAL_MAX)};
    rgb8_t rgb8;
    ColorUtil::fast_hsv2rgb_32bit(hsv16.h, hsv16.s, hsv16.v, &rgb8.r, &rgb8.g, &rgb8.b);
    //timer->add("::hsv2rgb(hsv)");
    
    changesArray[index]->combine(rgb8);

    //timer->add("changesArray[index]->combine(rgb)");

    //timer->print();
    //delete(timer);

    // Create the return value, for re-use
    irgb8_t irgb;
    irgb.rgb = rgb8;
    irgb.i = index;
    return irgb;
}

void LEDStrip::setRGBUnprotected(int index, rgb8_t rgb8){
#ifdef LEDS_OFFSET
    index = (index + LEDS_OFFSET) % NUM_LEDS;
    // Ensure index is positive
    if (index < 0) {
        index += NUM_LEDS;
    }
#endif
    // If we're in a performance-critical section where sanitization has already been done,
    // apply directly to the changes array
    changesArray[index]->combine(rgb8);
}



void LEDStrip::apply(){
    uint8_t r, g, b;
    rgb8_t rgb;
    float brightness = ledController->getBrightness();
    
    uint8_t brightnessLUT[256];
    uint32_t brightnessInt = (uint32_t)(brightness * 1000.0);
    ExecTimer *timer = new ExecTimer();
    timer->start("WithinApply");

    // for(int i = 0; i < 256; i++){
    //     brightnessLUT[i] = i * brightness;
    // }

    for(int i = 0; i < 256; i++){
        brightnessLUT[i] = (i*brightnessInt)/1000;
    }

    // brightnessLUT[0] = brightnessInt/1000;
    // for(int i = 1; i < 256; i++){
    //     brightnessLUT[i] = (brightnessLUT[i-1]*1000 + brightnessInt)/1000;
    // }

    timer->add("brightnessLUT[i] = i * brightness;");
    for (int i = 0; i < numLEDs; i++) {
        LEDChange* change = changesArray[i];
        if (change->count != 0) {
            rgb = change->getRGB();
            r = brightnessLUT[rgb.r];
            g = brightnessLUT[rgb.g];
            b = brightnessLUT[rgb.b];
            ledsArray[3*i] = g;
            ledsArray[3*i+1] = r;
            ledsArray[3*i+2] = b;
            change->count = 0;
        }
    }
    timer->add("LEDChange* change = changesArray[i];");
    //timer->print();
    delete(timer);
}


void LEDStrip::output(){
    ledController->outputLEDs(strip, ledsArray, numLEDs);
}

void LEDStrip::clear(){
    for(int i = 0; i < numLEDs; i++){
        ledsArray[i*3] = 0;
        ledsArray[i*3+1] = 0;
        ledsArray[i*3+2] = 0;
    }
}

void LEDStrip::giveController(Controller * ledController){
    this->ledController = ledController;
}

double LEDStrip::num(){ //Get num LEDs
    return (double)numLEDs;
} 
void LEDStrip::setNum(uint16_t num){ //set number of LEDs
    numLEDs = num;
    clear();
} 