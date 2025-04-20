#include "BounceBall.h"
#include "stdio.h"

void BounceBall::init(){

    blueprint.brightness = 1;
    blueprint.hue = hue;
    blueprint.index = 0;
    blueprint.Toffset = 0;
    blueprint.Trise = 0;
    blueprint.Thold = 0;
    blueprint.Tfall = 200;

    blueprint.exp_dropoff = 0.95;
    velTimer = new Timing();

    bounce_end = LEDs::strip(0)->num() - 1;    // 120 - 1 = 119
}

void BounceBall::run(){
    // Calculate the new position.
    float seconds_passed = velTimer->takeSeconds();
    uint16_t prev_pos = (uint16_t)pos;

    pos = pos + vel * seconds_passed;
    vel += gravity * seconds_passed;

    // Light up the LED where we are.
    SingleTime * new_light;
    if ((uint16_t)pos != prev_pos) {
        blueprint.index = pos;
        blueprint.hue += hue_shift;
        new_light = new SingleTime();
        new_light->init(blueprint);
        Effect::engine->queueApply(new_light);
    }
    
    // Handle bounce.
    if (pos >= bounce_end) {
        vel = -vel * bounce_factor;
        pos -= 1;

        // Handle removing.
        // if (abs(vel) < 0.001) {
        //     done = 1;
        // }
    }
}

void BounceBall::reset(int position, int direction, float brightness, float hue, float hue_shift, float speed){
    /*
     * The spark is a reusable object.
     * Get it ready for next time.
     */
    blueprint.index = position;
    this->direction = direction;
    blueprint.brightness = brightness;
    blueprint.hue = hue;
    this->hue_shift = hue_shift;
    this->vel = speed;
    blueprint.saturation = 1;
    blueprint.Tfall = 1200;
}


BounceBall::~BounceBall(){
    delete(velTimer);
    
}