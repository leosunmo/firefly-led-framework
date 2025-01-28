#include "GPIOInterruptHandler.h"
#include "hardware/gpio.h"

std::unordered_map<uint, std::vector<std::function<void()>>> GPIOInterruptHandler::pinCallbacks;

void GPIOInterruptHandler::registerCallback(uint gpio, std::function<void()> callback) {
    pinCallbacks[gpio].push_back(callback);
    gpio_set_irq_enabled_with_callback(gpio, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}

void GPIOInterruptHandler::handleInterrupt(uint gpio, uint32_t event) {
    if (pinCallbacks.find(gpio) != pinCallbacks.end()) {
        for (const auto& callback : pinCallbacks[gpio]) {
            callback();
        }
    }
}

extern "C" void gpio_callback(uint gpio, uint32_t event) {
    GPIOInterruptHandler::handleInterrupt(gpio, event);
}