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
    midpoint = NUM_LEDS/2+startOffset+centerOffset;
    for (int i=midpoint; i<(NUM_LEDS-endOffset);i++){
        rightRaindropEffect->pixels->push_back(Pixel(i));
    }
    for (int i=midpoint-1; i>=0+startOffset;i--) {
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

}

void Raindrop::run() {}

void Raindrop::release(){
    delete(myTiming);
    delete(rightRaindropEffect);
    delete(leftRaindropEffect);
}