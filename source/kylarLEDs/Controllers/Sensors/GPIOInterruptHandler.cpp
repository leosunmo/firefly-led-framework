#include "GPIOInterruptHandler.h"
#include "hardware/gpio.h"
#include <stdio.h>

std::unordered_map<uint, std::vector<std::function<void()>>> GPIOInterruptHandler::pinCallbacks;

void GPIOInterruptHandler::registerCallback(uint gpio, std::function<void()> callback, uint32_t event_mask)
{
    pinCallbacks[gpio].push_back(callback);
    printf("Initializing GPIO interrupt handler for GPIO %d with IRQ level %d\n", gpio, event_mask);
    gpio_set_irq_enabled_with_callback(gpio, event_mask, true, &gpio_callback);
}

void GPIOInterruptHandler::handleInterrupt(uint gpio, uint32_t event)
{
    if (pinCallbacks.find(gpio) != pinCallbacks.end())
    {
        for (const auto &callback : pinCallbacks[gpio])
        {
            callback();
        }
    }
    else
    {
        printf("No callbacks registered for GPIO %d\n", gpio);
    }
}

// Function to print all registered callbacks
void GPIOInterruptHandler::printCallbacks()
{
    for (const auto &[gpio, callbacks] : pinCallbacks)
    {
        printf("GPIO %d has %d callbacks\n", gpio, callbacks.size());
    }
}

extern "C" void gpio_callback(uint gpio, uint32_t event)
{
    GPIOInterruptHandler::handleInterrupt(gpio, event);
}