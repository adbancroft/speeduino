

/*
NOTE - This file and it's associated functions need a CLEARER NAME

//Purpose
We're implementing a lower frequency interrupt loop to perform calculations that are needed less often, some of which depend on time having passed (delta/time) to be meaningful.


//Technical
Timer2 is only 8bit so we are setting the prescaler to 128 to get the most out of it. This means that the counter increments every 0.008ms and the overflow at 256 will be after 2.048ms
Max Period = (Prescale)*(1/Frequency)*(2^8)
(See arduinomega.blogspot.com.au/2011/05/timer2-and-overflow-interrupt-lets-get.html)

We're after a 1ms interval so we'll need 131 intervals to reach this ( 1ms / 0.008ms per tick = 125).
Hence we will preload the timer with 131 cycles to leave 125 until overflow (1ms).

*/
#ifndef TIMERS_H
#define TIMERS_H

// #include "globals.h"
#include "pin_mapping.h"
#include "port_pin.h"
#include "board_selector.h"
#include "globals.h"

#define SET_COMPARE(compare, value) compare = (COMPARE_TYPE)(value) // It is important that we cast this to the actual overflow limit of the timer. The compare variables type can be bigger than the timer overflow.

/**
 * @brief Convert a frequency to timer ticks
 * 
 * @param frequency The frequency in Hz
 * @return uint16_t the equivalent numebr of ticks
 */
static inline uint16_t frequencyToTimerTicks(uint16_t frequency) {
  return (uint16_t)(MICROS_PER_SEC / (getTimerInterval() * frequency));
}

extern ioPort tach_pin_port;
#define TACHO_PULSE_HIGH() setPin_High(tach_pin_port)
#define TACHO_PULSE_LOW() setPin_Low(tach_pin_port)

enum TachoOutputStatus {TACHO_INACTIVE, READY, ACTIVE}; //The 3 statuses that the tacho output pulse can have. NOTE: Cannot just use 'INACTIVE' as this is already defined within the Teensy Libs
extern volatile TachoOutputStatus tachoOutputFlag;

#if defined (CORE_TEENSY)
  extern IntervalTimer lowResTimer;
  void oneMSInterval(void);
#elif defined (ARDUINO_ARCH_STM32)
  void oneMSInterval(void);
#endif
void initialiseTimers(const pin_mapping_t &pin);

#endif // TIMERS_H
