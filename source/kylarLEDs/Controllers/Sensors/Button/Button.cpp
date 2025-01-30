#include "Button.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include "stdio.h"

Button::Button(int pin) : pin(pin), last_time(get_absolute_time())
{
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
    GPIOInterruptHandler::registerCallback(pin, [this]()
                                           { this->handleInterrupt(); });
}

void Button::setCallback(std::function<void()> callback)
{
    Button::callbacks.push_back(callback);
}

void Button::clearCallbacks()
{
    Button::callbacks.clear();
}

void Button::handleInterrupt()
{
    static int press_state = 0; // 0 Waiting for press, 1 waiting for release
    absolute_time_t new_time = get_absolute_time();
    // Debounce
    if (absolute_time_diff_us(last_time, new_time) < 30000)
    {
        // 30ms debounce
        return;
    }

    Button::last_time = new_time;

    if (press_state == 0)
    {
        press_state = 1;
    }
    else
    {
        press_state = 0;
    }

    // Call all callbacks
    for (const auto &callback : Button::callbacks)
    {
        callback();
    }
}
