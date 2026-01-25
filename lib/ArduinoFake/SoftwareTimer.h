#pragma once

#if defined(NATIVE_BOARD)

#include <cstdint>
#include <atomic>

class software_timer_t {
public:

    using counter_t = uint32_t;
    using callback_t = void(*)();

    std::atomic<counter_t> counter = {0U};
    std::atomic<counter_t> compare = {0U};
    
    software_timer_t();

    void setCallback(callback_t callback);

    void enableTimer(void) { enabled = true; }
    void disableTimer(void) { enabled = false; }

private:
    // Atomic flag to signal the thread to stop
    std::atomic<bool> enabled = {false}; 
    std::atomic<callback_t> callback = {nullptr};
    
    void internalCallback(void);
};

#endif