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
#include "utilities.h"
#include "scheduledIO.h"

#define USE_IGN_REFRESH
#define IGNITION_REFRESH_THRESHOLD  30 //Time in uS that the refresh functions will check to ensure there is enough time before changing the end compare

/** @brief Initialize all schedulers to the OFF state */
void initialiseSchedulers(void);

/** @brief Start the timers that drive schedulers  */
void startSchedulers(void);

/** @brief Start fuel system  priming the fuel */
void beginInjectorPriming(void);

void disablePendingFuelSchedule(byte channel);
void disablePendingIgnSchedule(byte channel);

/** \enum ScheduleStatus
 * @brief The current state of a schedule
 * */
enum ScheduleStatus {
  // We use powers of 2 so we can check multiple states with a single bitwise AND

  /** Not running */
  OFF = 1, 
  /** The delay phase of the schedule is active */
  PENDING = 2,
  /** The schedule action is running */
  RUNNING = 4,
  /** The schedule is running, with a next schedule queued up */
  RUNNING_WITHNEXT = 8,
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
 * ISR.
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
  Schedule(counter_t &counter, compare_t &compare);

  /**
   * @brief Scheduled duration (timer ticks) 
   *
   * This captures the duration of the *next* interval to be scheduled. I.e.
   *  * Status==PENDING: this is the duration that will be used when the schedule moves to the RUNNING state 
   *  * Status==RUNNING_WITHNEXT: this is the duration that will be used after the current schedule finishes and the queued up scheduled starts 
   */
  volatile COMPARE_TYPE Duration;   ///< 
  volatile ScheduleStatus Status;   ///< Schedule status
  voidVoidCallback pStartCallback;  ///< Start Callback function for schedule
  voidVoidCallback pEndCallback;    ///< End Callback function for schedule

  volatile COUNTER_TYPE nextStartCompare;    ///< Planned start of next schedule (when current schedule is RUNNING_WITHNEXT)
  
  counter_t &_counter;       ///< **Reference** to the counter register. E.g. TCNT3
  compare_t &_compare;       ///< **Reference**to the compare register. E.g. OCR3A
};

/**
 * @brief Is the schedule running?
 * I.e. the action has started, but not finished. E.g. injector is open
 */
static inline bool isRunning(const Schedule &schedule) {
  // Using flags and bitwise AND (&) to check multiple states is much quicker
  // than a logical or (||) (one less branch & 30% less instructions)
  static constexpr uint8_t flags = RUNNING | RUNNING_WITHNEXT;
  return (bool)(schedule.Status & flags);
}

/// @cond 
// Private function - not for use external to the scheduler code

static inline void _setScheduleRunning(Schedule &schedule, uint32_t timeout, uint32_t duration)
{
  //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
  schedule.Duration = uS_TO_TIMER_COMPARE(duration);
  schedule._compare = schedule._counter + uS_TO_TIMER_COMPARE(timeout);
  schedule.Status = PENDING; //Turn this schedule on
}

static inline void _setScheduleNext(Schedule &schedule, uint32_t timeout, uint32_t duration)
{
   //If the schedule is already running, we can set the next schedule so it is ready to go
  //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
  schedule.nextStartCompare = schedule._counter + uS_TO_TIMER_COMPARE(timeout);
  // Schedule must already be running, so safe to reuse this.
  schedule.Duration = uS_TO_TIMER_COMPARE(duration);
  schedule.Status = RUNNING_WITHNEXT;
}

static inline  __attribute__((always_inline)) void _setSchedule(Schedule &schedule, uint32_t timeout, uint32_t duration, uint16_t maxAngle)
{
  if(likely(timeout < MAX_TIMER_PERIOD))
  {
    if (unlikely(duration > MAX_TIMER_PERIOD)) 
    {
      duration = MAX_TIMER_PERIOD - 1UL; //Safety check to ensure the duration is not longer than the maximum timer period
    }
    ATOMIC() 
    {
      if(!isRunning(schedule)) 
      { //Check that we're not already part way through a schedule
        _setScheduleRunning(schedule, timeout, duration);
      }
      // Check whether timeout exceeds the maximum future time. This can potentially occur on sequential setups when below ~115rpm
      else if(angleToTimeMicroSecPerDegree(maxAngle) < MAX_TIMER_PERIOD)
      {
        _setScheduleNext(schedule, timeout, duration);
      }
      else 
      {
        // Keep MISRA checker happy
      }
    }
  }
}

/// @endcond

/**
 * @brief Set the schedule callbacks. I.e the functions called when the action
 * needs to start & stop
 * 
 * @param schedule Schedule to modify
 * @param pStartCallback The new start callback - called when the schedule switches to RUNNING status
 * @param pEndCallback The new end callback - called when the schedule switches to from RUNNING to OFF status
 */
void setCallbacks(Schedule &schedule, voidVoidCallback pStartCallback, voidVoidCallback pEndCallback);


/** @brief An ignition schedule.
 *
 * Goal is to fire the spark as close to the requested angle as possible.
 * 
 * \code 
 *   <--------------- Delay ---------------><---- Charge Coil ---->
 *                                                                ^
 *                                                              Spark
 * \endcode
 * 
 * Terminology: dwell is the period when the ignition system applies an electric
 * current to the ignition coil's primary winding in order to charge up the coil
 * so it can generate a spark. 
 * 
 * Note that dwell times use uint16_t & therefore maximum dwell is 65.535ms. 
 * This limit is imposed elsewhere in Speeduino also.
 */
struct IgnitionSchedule : public Schedule {

  using Schedule::Schedule;

  volatile uint32_t _startTime; /**< The system time (in uS) that the schedule started, used by the overdwell protection in timers.ino */
  int16_t chargeAngle;        ///< Angle the coil should begin charging.
  int16_t dischargeAngle;          ///< Angle the coil should discharge at. I.e. spark.
  int16_t channelDegrees;    ///< The number of crank degrees until cylinder is at TDC  
};

/// @cond 
// Private functions - not for use external to the scheduler code

/** @brief Set the next schedule for the ignition channel.
 * 
 * @param schedule The ignition channel
 * @param delay The time to wait in µS until starting to charge the coil
 * @param dwellDuration The coil dwell time in µS
 */
static inline  __attribute__((always_inline)) void _setIgnitionScheduleDuration(IgnitionSchedule &schedule, uint32_t timeout, uint32_t duration) 
{
  _setSchedule(schedule, timeout, duration, CRANK_ANGLE_MAX_IGN);
}
/// @endcond

/**
 * @brief Check that no ignition channel has been charging the coil for too long
 * 
 * The over dwell protection system runs independently of the standard ignition 
 * schedules and monitors the time that each ignition output has been active. If the 
 * active time exceeds this amount, the output will be ended to prevent damage to coils.
 * 
 * @note Must be called once per millisecond by an **external** timer.
 */
void applyOverDwellProtection(void);


/**
 * @brief Shared ignition schedule timer ISR *implementation*. Should be called by the actual ignition timer ISRs
 * (as timed interrupts) when either the start time or the duration time are reached. See @ref schedule-state-machine
 * 
 * @param schedule The ignition schedule to move to the next state
 */
void moveToNextState(IgnitionSchedule &schedule);

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


/** Fuel injection schedule.
* Fuel schedules don't use the callback pointers, or the _startTime/endScheduleSetByDecoder variables.
* They are removed in this struct to save RAM.
*/
struct FuelSchedule : public Schedule {

  using Schedule::Schedule;

};

static inline  __attribute__((always_inline)) void setFuelSchedule(FuelSchedule &schedule, uint32_t timeout, uint32_t duration) 
{
  _setSchedule(schedule, timeout, duration, CRANK_ANGLE_MAX_INJ);
}

/**
 * @brief Shared fuel schedule timer ISR implementation. Should be called by the actual timer ISRs
 * (as timed interrupts) when either the start time or the duration time are reached. See @ref schedule-state-machine
 * 
 * @param schedule The fuel schedule to move to the next state
 */
void moveToNextState(FuelSchedule &schedule);

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

#endif // SCHEDULER_H
