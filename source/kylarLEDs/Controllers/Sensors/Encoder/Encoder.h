#pragma once
#include "stdint.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <functional>
#include <vector>
#include "../GPIOInterruptHandler.h"

class Encoder
{
public:
    Encoder(int A, int B);
    int getCount() const;
    void setAccumulate(bool accumulate) { this->accumulate = accumulate; }
    void setCallback(std::function<void(int)> callback);
    void clearCallbacks();

private:
    void handleRiseAInterrupt();
    void handleFallAInterrupt();
    uint8_t pinA;
    uint8_t pinB;
    int lock;
    uint64_t locktime0;
    absolute_time_t locktime;
    int lastB;
    bool accumulate = false; // if true, accumulate the count
    int count;
    std::vector<std::function<void(int)>> callbacks;
};