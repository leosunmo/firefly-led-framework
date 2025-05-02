#include "Encoder.h"
#include "hardware/gpio.h"
#include <stdio.h>

// Mostly taken from https://github.com/wd5gnr/MbedQuadratureEncoder/tree/main
// and the associated blog post https://hackaday.com/2022/04/20/a-rotary-encoder-how-hard-can-it-be/

// TODO(leo): It seems like it's not behaving correctly when you move it quickly in one direction
// and then change direction. This is presumably because of locktime and co. Proper hardware will probably fix this.

Encoder::Encoder(int A, int B) : pinA(A), pinB(B), count(0)
{
    gpio_init(A);
    gpio_init(B);
    gpio_set_dir(A, GPIO_IN);
    gpio_set_dir(B, GPIO_IN);
    gpio_pull_up(A);
    gpio_pull_up(B);
    GPIOInterruptHandler::registerCallback(A, [this]()
                                           { this->handleInterrupt(); }, GPIO_IRQ_EDGE_RISE);
}

int Encoder::getCount() const
{
    return Encoder::count;
}

void Encoder::setCallback(std::function<void(int)> callback)
{
    Encoder::callbacks.push_back(callback);
}

void Encoder::clearCallbacks()
{
    Encoder::callbacks.clear();
}

void Encoder::handleInterrupt()
{
    static absolute_time_t last_time = {0};
    static double velocity = 0; // measure how many times the encoder has been turned in the last second
    static int last_direction = 0; // 1 for positive, -1 for negative, 0 for no movement
    absolute_time_t new_time = get_absolute_time();

    if (absolute_time_diff_us(last_time, new_time) < 2000)
    { // 2ms debounce
        return;
    }

    uint64_t time_delta = absolute_time_diff_us(last_time, new_time);

    velocity = (1000000.0 / (double)time_delta);
    // 0 - 10 : intentional
    // 10-20 : normal
    // 20+ : fast
    int add = 1 + (int)velocity / 10; // scale 

    // Determine the current direction
    int current_direction = gpio_get(Encoder::pinB) ? 1 : -1;

    // Reset velocity if direction changes
    if (current_direction != last_direction && last_direction != 0)
    {
        velocity = 0;
    }

    last_direction = current_direction;
    last_time = new_time;

    if (Encoder::accumulate)
    {
        // If we are     // Update count based on direction
    Encoder::count += add * current_direction;
    }
    else
    {
        // If we are accumulating, add or subtract the count
        Encoder::count += (current_direction ? 1 : -1);
    }

    for (const auto &callback : Encoder::callbacks)
    {
        callback(Encoder::count);
    }
}