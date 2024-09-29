#include "Raindrop.h"
#include <stdio.h>
#include "../../Effects/SoundReactive/RaindropEffect.h"
#include "../../Effects/Effect.h"
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"

void Raindrop::init(){
    myTiming = new Timing();

    // initialize rightRaindropEffect and leftRaindropEffect
    rightRaindropEffect = new RaindropEffect();
    rightRaindropEffect->init();

    leftRaindropEffect = new RaindropEffect();
    leftRaindropEffect->init();

    // set the pixels each effect uses
    midpoint = NUM_LEDS/2;
    for (int i=midpoint; i<(NUM_LEDS);i++){
        rightRaindropEffect->pixels->push_back(Pixel(i));
    }
    for (int i=midpoint-1; i>=0;i--) {
        leftRaindropEffect->pixels->push_back(Pixel(i));
    }

    // used for the prototype
    if (prototype) {
        for (int i=4; i<9; i++) {
            rightRaindropEffect->pixels->push_back(Pixel(i));
        }
    }
    Effect::engine->apply(rightRaindropEffect);
    Effect::engine->apply(leftRaindropEffect);

    LEDs::useGlobalBrightnessControl(true, &(this->global_brightness));
}

void Raindrop::run() {
    double mic_brightness = (Microphone::getLowNormal() + Microphone::getHighNormal())/2.0;
    double new_brightness = mic_brightness * 0.3 + 0.7;
    if (new_brightness > global_brightness) {
        global_brightness = (new_brightness * 1.0 + global_brightness * 50.0)/51.0;;;
    } else {
        global_brightness = (new_brightness * 1.0 + global_brightness * 100.0)/101.0;;
    }
    
}

void Raindrop::release(){
    delete(myTiming);
    //delete(rightRaindropEffect); Don't need to delete the effects, the EffectEngine takes care of it :) -ky
    //delete(leftRaindropEffect);
}