/** @file
Injector and Ignition (on/off) scheduling (structs).

This scheduler is designed to maintain 2 schedules for use by the fuel and ignition systems.
It functions by waiting for the overflow vectors from each of the timers in use to overflow, which triggers an interrupt.

## Technical

Currently I am prescaling the 16-bit timers to 256 for injection and 64 for ignition.
This means that the counter increments every 16us (injection) / 4uS (ignition) and will overflow every 1048576uS.

    Max Period = (Prescale)*(1/Frequency)*(2^17)

For more details see https://playground.arduino.cc/Code/Timer1/ (OLD: http://playground.arduino.cc/code/timer1 ).
This means that the precision of the scheduler is:

- 16uS (+/- 8uS of target) for fuel
- 4uS (+/- 2uS) for ignition

## Features

This differs from most other schedulers in that its calls are non-recurring (ie when you schedule an event at a certain time and once it has occurred,
it will not reoccur unless you explicitly ask/re-register for it).
Each timer can have only 1 callback associated with it at any given time. If you call the setCallback function a 2nd time,
the original schedule will be overwritten and not occur.

## Timer identification

Arduino timers usage for injection and ignition schedules:
- timer3 is used for schedule 1(?) (fuel 1,2,3,4 ign 7,8)
- timer4 is used for schedule 2(?) (fuel 5,6 ign 4,5,6)
- timer5 is used ... (fuel 7,8, ign 1,2,3)

Timers 3,4 and 5 are 16-bit timers (ie count to 65536).
See page 136 of the processors datasheet: http://www.atmel.com/Images/doc2549.pdf .

256 prescale gives tick every 16uS.
256 prescale gives overflow every 1048576uS (This means maximum wait time is 1.0485 seconds).

*/
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <SimplyAtomic.h>
#include "globals.h"
#include "crankMaths.h"
#include "scheduledIO.h"

#define USE_IGN_REFRESH
#define IGNITION_REFRESH_THRESHOLD  30 //Time in uS that the refresh functions will check to ensure there is enough time before changing the end compare

#define DWELL_AVERAGE_ALPHA 30
#define DWELL_AVERAGE(input) LOW_PASS_FILTER((input), DWELL_AVERAGE_ALPHA, currentStatus.actualDwell)
//#define DWELL_AVERAGE(input) (currentStatus.dwell) //Can be use to disable the above for testing

void initialiseIgnitionSchedulers(void);

void startIgnitionSchedulers(void);
void stopIgnitionSchedulers(void);
void disablePendingIgnSchedule(byte channel);

void refreshIgnitionSchedule1(unsigned long timeToEnd);

void initialiseFuelSchedulers(void);

void startFuelSchedulers(void);
void stopFuelSchedulers(void);

void beginInjectorPriming(void);

void disablePendingFuelSchedule(byte channel);

//The ARM cores use separate functions for their ISRs
#if defined(ARDUINO_ARCH_STM32) || defined(CORE_TEENSY)
  void fuelSchedule1Interrupt(void);
  void fuelSchedule2Interrupt(void);
  void fuelSchedule3Interrupt(void);
  void fuelSchedule4Interrupt(void);
#if (INJ_CHANNELS >= 5)
  void fuelSchedule5Interrupt(void);
#endif
#if (INJ_CHANNELS >= 6)
  void fuelSchedule6Interrupt(void);
#endif
#if (INJ_CHANNELS >= 7)
  void fuelSchedule7Interrupt(void);
#endif
#if (INJ_CHANNELS >= 8)
  void fuelSchedule8Interrupt(void);
#endif
#if (IGN_CHANNELS >= 1)
  void ignitionSchedule1Interrupt(void);
#endif
#if (IGN_CHANNELS >= 2)
  void ignitionSchedule2Interrupt(void);
#endif
#if (IGN_CHANNELS >= 3)
  void ignitionSchedule3Interrupt(void);
#endif
#if (IGN_CHANNELS >= 4)
  void ignitionSchedule4Interrupt(void);
#endif
#if (IGN_CHANNELS >= 5)
  void ignitionSchedule5Interrupt(void);
#endif
#if (IGN_CHANNELS >= 6)
  void ignitionSchedule6Interrupt(void);
#endif
#if (IGN_CHANNELS >= 7)
  void ignitionSchedule7Interrupt(void);
#endif
#if (IGN_CHANNELS >= 8)
  void ignitionSchedule8Interrupt(void);
#endif
#endif

/** \enum ScheduleStatus
 * @brief The current state of a schedule
 * */
enum ScheduleStatus {
  // We use powers of 2 so we can check multiple states with a single bitwise AND

  /** Not running */
  OFF              = 0b00000000U, 
  /** The delay phase of the schedule is active */
  PENDING          = 0b00000001U,
  /** The schedule action is running */
  RUNNING          = 0b00000010U,
  /** The schedule is running, with a next schedule queued up */
  RUNNING_WITHNEXT = 0b00000100U,
}; 


/**
 * @brief A schedule for a single output channel. 
 * 
 * @details
 * @par A schedule consists of 3 independent parts:
 * - an action that can be started and stopped. E.g. charge ignition coil or injection pulse
 * - a delay until the action is started
 * - a duration until the action is stopped
 * 
 * I.e.\n 
 * \code 
 *   <--------------- Delay ---------------><---- Duration ---->
 *                                          ^                  ^
 *                              Action starts                  Action ends
 * \endcode
 * 
 * @par Timers are modelled as registers
 * Once set, Schedule instances are usually driven externally by a timer
 * ISR calling moveToNextState()
 */
struct Schedule {
  // Deduce the real types of the counter and compare registers.
  // COMPARE_TYPE is NOT the same - it's just an integer type wide enough to
  // store 16-bit counter/compare calculation results.
  /** @brief The type of a timer counter register (this varies between platforms) */
  using counter_t = decltype(FUEL1_COUNTER /* <-- Arbitrary choice of macro, assumes all have the same type */);
  /** @brief The type of a timer compare register (this varies between platforms) */
  using compare_t = decltype(FUEL1_COMPARE /* <-- Arbitrary choice of macro, assumes all have the same type */);

  /**
   * @brief Construct a new Schedule object
   * 
   * @param counter A <b>reference</b> to the timer counter
   * @param compare A <b>reference</b> to the timer comparator
   */
  constexpr Schedule(counter_t &counter, compare_t &compare)
    : _counter(counter)
    , _compare(compare) 
  {
  }  

  /**
   * @brief Scheduled duration (timer ticks) 
   *
   * This captures the duration of the *next* interval to be scheduled. I.e.
   *  * Status==PENDING: this is the duration that will be used when the schedule moves to the RUNNING state 
   *  * Status==RUNNING_WITHNEXT: this is the duration that will be used after the current schedule finishes and the queued up scheduled starts 
   */
  volatile COMPARE_TYPE duration = 0U;  ///< Scheduled duration (uS ?)
  volatile ScheduleStatus Status = OFF;  ///< Schedule status: OFF, PENDING, STAGED, RUNNING
  voidVoidCallback pStartCallback = &nullCallback; ///< Start Callback function for schedule
  voidVoidCallback pEndCallback = &nullCallback;   ///< End Callback function for schedule
  COMPARE_TYPE nextStartCompare = 0U;   ///< Planned start of next schedule (when current schedule is RUNNING)
  
  counter_t &_counter;       ///< **Reference** to the counter register. E.g. TCNT3
  compare_t &_compare;       ///< **Reference**to the compare register. E.g. OCR3A
};

static inline bool isRunning(const Schedule &schedule) {
  // Using flags and bitwise AND (&) to check multiple states is much quicker
  // than a logical or (||) (one less branch & 30% less instructions)
  static constexpr uint8_t flags = RUNNING | RUNNING_WITHNEXT;
  return ((uint8_t)schedule.Status & flags)!=0U;
}

static void inline _setDuration(Schedule &schedule, uint16_t duration) {
#ifndef MAX_TIMER_PERIOD
  #error MAX_TIMER_PERIOD must be defined
#else
#if MAX_TIMER_PERIOD < UINT16_MAX //cppcheck-suppress misra-c2012-20.9
  schedule.duration = (COMPARE_TYPE)uS_TO_TIMER_COMPARE(min(MAX_TIMER_PERIOD - 1U, duration));
#else
  schedule.duration = (COMPARE_TYPE)uS_TO_TIMER_COMPARE(duration);
#endif
#endif
}

static inline __attribute__((always_inline)) void _setScheduleNext(Schedule &schedule, uint32_t timeout, uint16_t duration)
{
  //The duration of the pulsewidth cannot be longer than the maximum timer period. This is unlikely as pulse widths should never get that long, but it's here for safety
  //Duration can safely be set here as the schedule is already running at the previous duration value already used
  _setDuration(schedule, duration);
  schedule.nextStartCompare = schedule._counter + (COMPARE_TYPE)uS_TO_TIMER_COMPARE(timeout);
  schedule.Status = RUNNING_WITHNEXT;
}

static inline __attribute__((always_inline)) void _setScheduleRunning(Schedule &schedule, uint32_t timeout, uint16_t duration)
{
  //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
  //The duration of the pulsewidth cannot be longer than the maximum timer period. This is unlikely as pulse widths should never get that long, but it's here for safety
  _setDuration(schedule, duration);
  COMPARE_TYPE startCompare = schedule._counter + (COMPARE_TYPE)uS_TO_TIMER_COMPARE(timeout);
  schedule._compare = startCompare;
  schedule.Status = PENDING; //Turn this schedule on
}

void setCallbacks(Schedule &schedule, voidVoidCallback pStartCallback, voidVoidCallback pEndCallback);


/** Ignition schedule.
 */
struct IgnitionSchedule : public Schedule {

  using Schedule::Schedule;

  volatile COMPARE_TYPE endCompare;   ///< The counter value of the timer when this will end
  volatile unsigned long startTime; /**< The system time (in uS) that the schedule started, used by the overdwell protection in timers.ino */
  volatile bool endScheduleSetByDecoder = false;
};

static inline __attribute__((always_inline)) void setIgnitionSchedule(IgnitionSchedule &schedule, uint32_t timeout, uint16_t duration) 
{
  if(timeout < MAX_TIMER_PERIOD)
  {
    ATOMIC() 
    {
      if(!isRunning(schedule)) 
      { //Check that we're not already part way through a schedule
        _setScheduleRunning(schedule, timeout, duration);
      }
      // Check whether timeout exceeds the maximum future time. This can potentially occur on sequential setups when below ~115rpm
      else if(angleToTimeMicroSecPerDegree((uint16_t)CRANK_ANGLE_MAX_IGN) < MAX_TIMER_PERIOD)
      {
        _setScheduleNext(schedule, timeout, duration);
      }
    }
  }
}

/** Fuel injection schedule.
* Fuel schedules don't use the callback pointers, or the startTime/endScheduleSetByDecoder variables.
* They are removed in this struct to save RAM.
*/
struct FuelSchedule : public Schedule {

  using Schedule::Schedule;

};

static inline __attribute__((always_inline)) void setFuelSchedule(FuelSchedule &schedule, uint32_t timeout, uint16_t duration) 
{
  if(timeout < MAX_TIMER_PERIOD)
  {
    ATOMIC() 
    {
      if(!isRunning(schedule)) 
      { //Check that we're not already part way through a schedule
        _setScheduleRunning(schedule, timeout, duration);
      }
      //If the schedule is already running, we can queue up the next pulse. Only do this however if the maximum time between pulses (Based on CRANK_ANGLE_MAX_INJ) is less than the max timer period
      else if(angleToTimeMicroSecPerDegree((uint16_t)CRANK_ANGLE_MAX_INJ) < MAX_TIMER_PERIOD) 
      {
        _setScheduleNext(schedule, timeout, duration);
      }
    }
  }
}

extern FuelSchedule fuelSchedule1;
extern FuelSchedule fuelSchedule2;
extern FuelSchedule fuelSchedule3;
extern FuelSchedule fuelSchedule4;
#if INJ_CHANNELS >= 5
extern FuelSchedule fuelSchedule5;
#endif
#if INJ_CHANNELS >= 6
extern FuelSchedule fuelSchedule6;
#endif
#if INJ_CHANNELS >= 7
extern FuelSchedule fuelSchedule7;
#endif
#if INJ_CHANNELS >= 8
extern FuelSchedule fuelSchedule8;
#endif

extern IgnitionSchedule ignitionSchedule1;
extern IgnitionSchedule ignitionSchedule2;
extern IgnitionSchedule ignitionSchedule3;
extern IgnitionSchedule ignitionSchedule4;
extern IgnitionSchedule ignitionSchedule5;
#if IGN_CHANNELS >= 6
extern IgnitionSchedule ignitionSchedule6;
#endif
#if IGN_CHANNELS >= 7
extern IgnitionSchedule ignitionSchedule7;
#endif
#if IGN_CHANNELS >= 8
extern IgnitionSchedule ignitionSchedule8;
#endif

#endif // SCHEDULER_H
