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
    num_steps = 10;
    brightness_loss = blueprint.brightness / num_steps;
}

void Spark::run(){
    if(initialized == 0){
        init();
    }

    if(num_steps <= 0){
        return;
    }
    
    if(timer->everyMs((uint32_t)(150.0/speed))){
        // "We have moved!"
        blueprint.index += direction;
        //Effect::engine->queueApply((new SingleTime())->init(blueprint));
        //Triangle* triangle = new Triangle(4000);
        //WaveformLED* wvfrm = new WaveformLED(triangle);
        //wvfrm->setI(blueprint.index)->setHue(blueprint.hue)->setCount(1);
        
        SingleTime* light = new SingleTime();
        light->init(blueprint);
        Effect::engine->queueApply(light);
        blueprint.brightness -= brightness_loss;
        blueprint.hue += hue_shift;
        blueprint.saturation += brightness_loss/2.0;
        num_steps--;
    }
    
    
    
}

void Spark::reset(int position, int direction, double brightness, int steps, double hue, double hue_shift, double speed){
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
    blueprint.saturation = 1.1-blueprint.brightness/5.0;
    blueprint.Tfall = 450.0 / speed; 
}

Spark::~Spark(){
    done = 1;
    delete(timer);
}