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
    bounce_end = LEDs::strip(0)->num() - 1;
}

void BounceBall::run(){
    // Calculate the new position.
    float seconds_passed = velTimer->takeSeconds();
    uint16_t prev_pos = (uint16_t)pos;

    pos = pos + vel * seconds_passed;
    vel += gravity * seconds_passed;
    // Ensure velocity is always treated as positive for calculations
    float abs_vel = std::abs(vel);

    // Light up the LEDs between the previous position and the current position.
    if ((uint16_t)pos != prev_pos) {
        int start = std::min(prev_pos, (uint16_t)pos);
        int end = std::max(prev_pos, (uint16_t)pos);

        for (int i = start; i <= end; ++i) {
            blueprint.index = i;
            blueprint.hue += hue_shift;
            SingleTime *new_light = new SingleTime();

            // Update Tfall calculation to use absolute velocity
            blueprint.Tfall = std::max(200.0f, 5000.0f / (abs_vel + 10.0f));

            new_light->init(blueprint);
            Effect::engine->queueApply(new_light);
        }
    }
    
    // Handle bounce logic with absolute velocity
    if ((direction == 1 && pos >= bounce_end) || (direction == 0 && pos <= bounce_end)) {
        vel = -vel * bounce_factor; // Reverse velocity direction
        pos = bounce_end; // Ensure the position stays within bounds

        // Handle Removing.
        // If the ball is moving slowly enough, we can remove it.
        // If it's not bouncing it might just be out of bounds, remove it.
        if (bounce_factor < 0.01 && (pos >= bounce_end || pos <= 0)) {
            done = 1;
        } else if (abs_vel < 0.01) {
            // If the ball is moving slowly enough, we can remove it.
            done = 1;
        }
    }
}

void BounceBall::reset(int position, int direction, float brightness, float hue, float hue_shift, float speed){
    if (direction == 1) {
        // Ball heading right, from 0 to num_leds
        bounce_end = static_cast<int>(LEDs::strip(0)->num())-1;
        this->vel = std::abs(speed); // Ensure velocity is positive for direction 1
        pos = position;
    } else {
        // Ball heading left from num_leds to 0
        this->vel = -std::abs(speed); // Ensure velocity is negative for direction 0
        gravity = -std::abs(gravity); // Ensure gravity is negative for direction 0
        bounce_end = 0;
        pos = position;
    }
    blueprint.index = position;
    this->direction = direction;
    blueprint.brightness = brightness;
    blueprint.hue = hue;
    this->hue_shift = hue_shift;
    blueprint.saturation = 1;
    blueprint.Tfall = 1200;
}


BounceBall::~BounceBall(){
    delete(velTimer);
}