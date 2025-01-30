#pragma once
#include "stdint.h"
#include "pico/stdlib.h"
#include <functional>
#include <unordered_map>
#include <vector>

class GPIOInterruptHandler {
public:
    static void registerCallback(uint gpio, std::function<void()> callback, uint32_t event_mask = GPIO_IRQ_EDGE_FALL);
    static void handleInterrupt(uint gpio, uint32_t event);
    static void printCallbacks();

private:
    static std::unordered_map<uint, std::vector<std::function<void()>>> pinCallbacks;
};

extern "C" void gpio_callback(uint gpio, uint32_t event);