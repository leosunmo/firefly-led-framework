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
    void setCallback(std::function<void(int)> callback);

private:
    void handleInterrupt();
    uint8_t pinA;
    uint8_t pinB;
    int count;
    std::vector<std::function<void(int)>> callbacks;
};