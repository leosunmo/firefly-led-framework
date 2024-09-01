#include "Gradient.h"
#include "stdio.h"

void Gradient::init(){
    for(int i = 0; i < NUM_LEDS; i++){
        last_value[i] = 0.0;
    }
}

void Gradient::run(){
    hsv_t color = {1, 1, brightness};
    for(int i = start; i <= stop; i++){
        double progress = (i - start) / (double)(stop - start);
        double offset = start_hue;
        double range = stop_hue - start_hue;
        if(start_hue < stop_hue){
            // Normal, hue goes from 0 -> 1
            
        }else{
            // Reversed, hue goes from 1 -> 0
            progress = 1.0 - progress;
            offset = stop_hue;
            range *= -1;

        }
        
        color.h = (range)*( progress ) + offset;
        color.v = (last_value[i]*16 + (rand()%100/750.0))/17.0;
        last_value[i] = color.v;
        //color.v += brightness;
        LEDs::setHSV(i, color);
    }
    
}

Gradient::~Gradient(){
    
}