#include "Spark.h"
#include <stdio.h>
#include "../Basics/WaveformLED.h"
#include "../../kylarLEDs/Utility/Waveforms/Triangle.h"

void Spark::init(){
    speed = 1.0;
    position = 0;
    blueprint.brightness = 1;
    blueprint.hue = hue;
    blueprint.index = 0;
    blueprint.Toffset = 0;
    blueprint.Trise = 0;
    blueprint.Thold = 0;
    blueprint.Tfall = 200;
    timer = new Timing();
    initialized = 1;
    num_steps = 0;
    brightness_loss = blueprint.brightness / num_steps;
}

void Spark::run(){
    if(initialized == 0){
        init();
    }

    if(total_steps > NUM_LEDS / 2){
        // hard limit, will not go past half the strip.
        return;
    }
    if(num_steps <= 0){
        return;
    }
    
    if(timer->everyMs((uint32_t)(130.0/speed))){
        // "We have moved!"
        blueprint.index += direction;
        //Effect::engine->queueApply((new SingleTime())->init(blueprint));
        //Triangle* triangle = new Triangle(4000);
        //WaveformLED* wvfrm = new WaveformLED(triangle);
        //wvfrm->setI(blueprint.index)->setHue(blueprint.hue)->setCount(1);
        
        SingleTime* light = new SingleTime();
        light->init(blueprint);
        Effect::engine->queueApply(light);
        
        if(blueprint.hue > 1){
            // don't go below 1
            blueprint.hue += hue_shift;
        }
        blueprint.saturation += brightness_loss/2.0;
        // Take from bonus steps first
        if(bonus_steps > 0){
            bonus_steps--;
        }else{
            blueprint.brightness -= brightness_loss;
            num_steps--;
        }
        
    }
    
    
    
}

void Spark::reset(int position, int direction, float brightness, int steps, float hue, float hue_shift, float speed){
    /*
     * The spark is a reusable object.
     * Get it ready for next time.
     */
    blueprint.index = position;
    this->direction = direction;
    blueprint.brightness = brightness;
    this->num_steps = steps;
    blueprint.hue = hue;
    this->hue_shift = hue_shift;
    this->brightness_loss = this->blueprint.brightness / (double)this->num_steps;
    this->speed = speed;
    blueprint.saturation = 1-blueprint.brightness/6.0;
    blueprint.Tfall = 1200.0 / speed; 
    bonus_steps = 0;
    bonused = 0;
    total_steps = 0;
}

Spark::~Spark(){
    done = 1;
    delete(timer);
}