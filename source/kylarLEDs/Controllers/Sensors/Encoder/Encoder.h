#pragma once
#include "stdint.h"
#include <limits>
#include "pico/stdlib.h"
#include "pico/time.h"
#include <functional>
#include <vector>
#include "../GPIOInterruptHandler.h"

class Encoder
{
public:
    Encoder(int A, int B);
    int minValue = std::numeric_limits<int>::min();
    int maxValue = std::numeric_limits<int>::max();
    bool wrapAround = false;
    
    int getCount() const;
    void setAccumulate(bool accumulate) { this->accumulate = accumulate; }
    void setRange(int min, int max, bool wrap = false) {
        minValue = min;
        maxValue = max;
        wrapAround = wrap;
    }
    void setCallback(std::function<void(int)> callback);
    void clearCallbacks();

private:
    // void handleRiseAInterrupt();
    // void handleFallAInterrupt();
    void handleInterrupt();
    uint8_t pinA;
    uint8_t pinB;
    bool accumulate = false; // if true, accumulate the count
    int count;
    std::vector<std::function<void(int)>> callbacks;
};