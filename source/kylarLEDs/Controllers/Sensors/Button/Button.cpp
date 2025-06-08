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
    // Register for both rising and falling edge interrupts (press and release)
    GPIOInterruptHandler::registerCallback(pin, [this]()
                                           { this->handleInterrupt(); },
                                           GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);
}

void Button::setCallback(std::function<void(int)> callback)
{
    Button::callbacks.push_back(callback);
}

void Button::clearCallbacks()
{
    Button::callbacks.clear();
}

void Button::handleInterrupt()
{
    absolute_time_t new_time = get_absolute_time();
    
    // Debounce
    if (absolute_time_diff_us(last_time, new_time) < 30000)
    {
        // 30ms debounce
        return;
    }

    Button::last_time = new_time;
    
    // Read the actual GPIO pin state
    // Since we use pull-up resistors:
    // - GPIO value = 0 means button is pressed (connected to ground)
    // - GPIO value = 1 means button is released (pulled up by resistor)
    uint8_t gpio_state = gpio_get(pin);
    
    // Convert to our button state convention: 
    // - 1 means pressed
    // - 0 means released
    int button_state = gpio_state ? 1 : 0;
    
    // Only call callbacks if there's a change
    static int last_reported_state = -1; // Initialize with invalid state
    
    // Only trigger callbacks if the state has changed
    if (button_state != last_reported_state) {
        last_reported_state = button_state;
        
        // Call all callbacks with the current button state
        for (const auto &callback : Button::callbacks)
        {
            callback(button_state);
        }
    }
}
