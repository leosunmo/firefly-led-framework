#pragma once
#include "../Pattern.h"
#include "../../kylarLEDs/Utility/Timing.h"
#include "../../kylarLEDs/Input/InputManager.h"
#include "../../Effects/SoundReactive/FullBar.h"
#include "../../Effects/SoundReactive/BounceBall.h"
#include <vector>
class ShakeelFlashBall : public Pattern{

    public:
        // New constructor with no parameters
        ShakeelFlashBall() : Pattern() {}
        
        void run();
        void init();
        void release();
    private:
        // baseSpeed is the variable baseline speed set by the encoder
        double baseSpeed = 0.0;

        double prevBaseSpeed = 0.0;
        // ballDirection is the direction of the ball
        // 0/false = left, 1/true = right
        bool ballDirection = true;
        // punch is an effect booster that is triggered for a short time
        // when the button is pressed
        bool punch = false;

        double baseBrightness = 0.9; // Adjust this value to set the minimum brightness level

        double ballTriggerThreshold = 0.3; // Threshold for triggering a new ball

        FullBar * bar;
        Timing *punchTimer;
};