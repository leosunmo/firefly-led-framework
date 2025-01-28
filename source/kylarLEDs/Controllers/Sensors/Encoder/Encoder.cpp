#include "Encoder.h"
#include "hardware/gpio.h"
#include <stdio.h>

Encoder::Encoder(int A, int B) : pinA(A), pinB(B), count(0)
{
    gpio_init(A);
    gpio_init(B);
    gpio_set_dir(A, GPIO_IN);
    gpio_set_dir(B, GPIO_IN);
    gpio_pull_up(A);
    gpio_pull_up(B);
    GPIOInterruptHandler::registerCallback(A, [this]()
                                           { this->handleInterrupt(); });
}

int Encoder::getCount() const
{
    return Encoder::count;
}

void Encoder::setCallback(std::function<void(int)> callback)
{
    Encoder::callbacks.push_back(callback);
}

void Encoder::handleInterrupt()
{
    static absolute_time_t last_time = {0};
    absolute_time_t new_time = get_absolute_time();

    // Debounce
    if (absolute_time_diff_us(last_time, new_time) < 2000)
    {
        // 2ms debounce
        return;
    }

    uint64_t time_delta = absolute_time_diff_us(last_time, new_time);
    double velocity = 1000000.0 / static_cast<double>(time_delta);
    int add = 1 + static_cast<int>(velocity) / 10;

    last_time = new_time;

    if (gpio_get(Encoder::pinB))
    {
        Encoder::count += add;
    }
    else
    {
        Encoder::count -= add;
    }

    for (const auto &callback : Encoder::callbacks)
    {
        callback(Encoder::count);
    }
}