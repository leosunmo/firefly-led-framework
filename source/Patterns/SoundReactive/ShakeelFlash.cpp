#include "ShakeelFlash.h"
#include <stdio.h>
#include <math.h>
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"

void ShakeelFlash::init()
{
    printf("Initialized ShakeelFlash\n");
    bar = new FullBar();
    bar->init();
}

void ShakeelFlash::run()
{
    double micVal = pow(Microphone::getLowNormal(), 2);
    bar->micVal = micVal;
    bar->run();
}

void ShakeelFlash::release()
{
    delete bar;
}