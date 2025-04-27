#pragma once
#include "../Pattern.h"
#include "../../kylarLEDs/Utility/Timing.h"
#include "../../kylarLEDs/Controllers/Sensors/Encoder/Encoder.h"
#include "../../kylarLEDs/Controllers/Sensors/Button/Button.h"
#include "../../Effects/SoundReactive/FullBar.h"
#include "../../Effects/SoundReactive/BounceBall.h"
#include <vector>
class ShakeelFlashBall : public Pattern{

    public:
        ShakeelFlashBall(Encoder *effectEncoder, Button *effectButton) : Pattern(), effectEncoder(effectEncoder), effectButton(effectButton) {}
        using Pattern::Pattern;
        void run();
        void init();
        void release();

        Encoder *effectEncoder;
        Button *effectButton;
    private:
        // baseSpeed is the variable baseline speed set by the encoder
        double baseSpeed = 0.0;
        // ballDirection is the direction of the ball
        // 0/false = left, 1/true = right
        bool ballDirection = true;
        // punch is an effect booster that is triggered for a short time
        // when the button is pressed
        bool punch = false;

        double baseBrightness = 0.9; // Adjust this value to set the minimum brightness level

        FullBar * bar;
        Timing *punchTimer;
};