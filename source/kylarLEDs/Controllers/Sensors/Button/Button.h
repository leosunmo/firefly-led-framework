#pragma once
#include "stdint.h"
#include "pico/stdlib.h"
#include <functional>
#include <vector>
#include "../GPIOInterruptHandler.h"

class Button
{
public:
    Button(int pin);
    void setCallback(std::function<void()> callback);

private:
    void handleInterrupt();
    uint8_t pin; // GPIO pin
    static std::vector<std::function<void()>> callbacks;
};