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
    lock = 0; // unlocked
    // default options
    locktime0 = 100000; // 100ms
    locktime = get_absolute_time() - locktime0;
    lastB = 2; // not 0 or 1
    GPIOInterruptHandler::registerCallback(A, [this]()
                                           { this->handleFallAInterrupt(); }, GPIO_IRQ_EDGE_FALL);
    GPIOInterruptHandler::registerCallback(A, [this]()
                                           { this->handleRiseAInterrupt(); }, GPIO_IRQ_EDGE_RISE);
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

// The encoder probably has a pull up so high is the rest state
// So when going back to rest, we look if we should reset the lock
// If b!=lastB then we unlock and set a new lastB
// If you have bounce noise, keep in mind that B should be stable while A is bouncing
// So once you remember the new lastB, this function does nothing
// since you will be unlocked and will set lastB=b which is a NOP
// However, we set lock=0 and lastB=b because it is cheaper than
// deciding which one to do, if any in the case where you should unlock
// or should remember a new lastB
void Encoder::handleRiseAInterrupt()
{
    // Read the state of pinB
    int b = gpio_get(Encoder::pinB);

    if (Encoder::lock && Encoder::lastB == b)
        return; // not time to unlock
    // if lock=0 and lastB==b these two lines do nothing
    // but if lock is 1 and/or lastB!=b then one of them does something
    Encoder::lock = 0;
    Encoder::lastB = b;
    Encoder::locktime = get_absolute_time() + Encoder::locktime0; // even if not locked, timeout the lastB
}

// The falling edge is where we do the count
// Note that if you pause a bit, the lock will expire because otherwise
// we have to monitor B also to know if a change in direction occured
// It is tempting to try to mutually lock/unlock the ISRs, but in real life
// the edges are followed by a bunch of bounce edges while B is stable
// B will change while A is stable
// So unless you want to also watch B against A, you have to make some
// compromise and this works well enough in practice
void Encoder::handleFallAInterrupt()
{
    int b;
    // clear lock if timedout and in either case forget lastB if we haven't seen an edge in a long time
    if (Encoder::locktime < get_absolute_time())
    {
        Encoder::lock = 0;
        Encoder::lastB = 2; // impossible value so we must read this event
    }
    if (lock)
        return;                  // we are locked so done
    b = gpio_get(Encoder::pinB); // read B
    if (b == Encoder::lastB)
        return;                                                   // no change in B
    Encoder::lock = 1;                                            // don't read the upcoming bounces
    Encoder::locktime = get_absolute_time() + Encoder::locktime0; // set up timeout for lock
    Encoder::lastB = b;                                           // remember where B is now


    if (!Encoder::accumulate)
    {
        // If we are not accumulating, just set count to 1 or -1
        Encoder::count = (b ? -1 : 1);
    }
    else
    {
        // If we are accumulating, add or subtract the count
        Encoder::count += (b ? -1 : 1);
    }

    for (const auto &callback : Encoder::callbacks)
    {
        callback(Encoder::count);
    }
}

// void Encoder::handleInterrupt()
// {
//     absolute_time_t new_time = get_absolute_time();
//     uint64_t time_delta = absolute_time_diff_us(last_time, new_time);
//     // Debounce
//     if (time_delta < 10000)
//     {
//         // 10ms debounce
//         return;
//     }

//     // double velocity = 1000000.0 / static_cast<double>(time_delta);
//     // int add = 1 + static_cast<int>(velocity / 10);
//     int add = 1;
//     Encoder::last_time = new_time;

//     if (gpio_get(Encoder::pinB))
//     {
//         Encoder::count += add;
//     }
//     else
//     {
//         Encoder::count -= add;
//     }

//     for (const auto &callback : Encoder::callbacks)
//     {
//         callback(Encoder::count);
//     }
// }