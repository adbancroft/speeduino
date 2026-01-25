#if defined(NATIVE_BOARD)
#include <thread>
#include <chrono>
#include <functional>
#include <Arduino.h>
#include "SoftwareTimer.h"

// This could definitely be made more efficient, but for testing purposes this will do

software_timer_t::software_timer_t()
{
    std::thread t([=]() { internalCallback(); });
    t.detach(); // Detach the thread to run independently
}

void software_timer_t::setCallback(callback_t cb)
{
    callback = cb;
}

void software_timer_t::internalCallback(void)
{
    while (true) {
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
        counter = static_cast<counter_t>(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count()); 
        if (enabled.load() && (callback != nullptr) && (compare!=0U) && (counter>=compare)) {
            callback.load()();
        }
    }
}

#endif