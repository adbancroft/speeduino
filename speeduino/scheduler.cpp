/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
/** @file
 * Injector and Ignition (on/off) scheduling (functions).
 * There is usually 8 functions for cylinders 1-8 with same naming pattern.
 * 
 * ## Scheduling structures
 * 
 * Structures @ref FuelSchedule and @ref Schedule describe (from scheduler.h) describe the scheduling info for Fuel and Ignition respectively.
 * They contain duration, current activity status, start timing, end timing, callbacks to carry out action, etc.
 * 
 * ## Scheduling Functions
 * 
 * For Injection:
 * - setFuelSchedule*(tout,dur) - **Setup** schedule for (next) injection on the channel
 * - inj*StartFunction() - Execute **start** of injection (Interrupt handler)
 * - inj*EndFunction() - Execute **end** of injection (interrupt handler)
 * 
 * For Ignition (has more complex schedule setup):
 * - setIgnitionSchedule*(cb_st,tout,dur,cb_end) - **Setup** schedule for (next) ignition on the channel
 * - ign*StartFunction() - Execute **start** of ignition (Interrupt handler)
 * - ign*EndFunction() - Execute **end** of ignition (Interrupt handler)
 */
#include "globals.h"
#include "scheduler.h"
#include "scheduledIO.h"
#include "timers.h"
#include "schedule_state_machine.h"

FuelSchedule fuelSchedule1(FUEL1_COUNTER, FUEL1_COMPARE); //cppcheck-suppress misra-c2012-8.4
FuelSchedule fuelSchedule2(FUEL2_COUNTER, FUEL2_COMPARE); //cppcheck-suppress misra-c2012-8.4
FuelSchedule fuelSchedule3(FUEL3_COUNTER, FUEL3_COMPARE); //cppcheck-suppress misra-c2012-8.4
FuelSchedule fuelSchedule4(FUEL4_COUNTER, FUEL4_COMPARE); //cppcheck-suppress misra-c2012-8.4
#if (INJ_CHANNELS >= 5)
FuelSchedule fuelSchedule5(FUEL5_COUNTER, FUEL5_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if (INJ_CHANNELS >= 6)
FuelSchedule fuelSchedule6(FUEL6_COUNTER, FUEL6_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if (INJ_CHANNELS >= 7)
FuelSchedule fuelSchedule7(FUEL7_COUNTER, FUEL7_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if (INJ_CHANNELS >= 8)
FuelSchedule fuelSchedule8(FUEL8_COUNTER, FUEL8_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif

IgnitionSchedule ignitionSchedule1(IGN1_COUNTER, IGN1_COMPARE); //cppcheck-suppress misra-c2012-8.4
IgnitionSchedule ignitionSchedule2(IGN2_COUNTER, IGN2_COMPARE); //cppcheck-suppress misra-c2012-8.4
IgnitionSchedule ignitionSchedule3(IGN3_COUNTER, IGN3_COMPARE); //cppcheck-suppress misra-c2012-8.4
IgnitionSchedule ignitionSchedule4(IGN4_COUNTER, IGN4_COMPARE); //cppcheck-suppress misra-c2012-8.4
IgnitionSchedule ignitionSchedule5(IGN5_COUNTER, IGN5_COMPARE); //cppcheck-suppress misra-c2012-8.4
#if IGN_CHANNELS >= 6
IgnitionSchedule ignitionSchedule6(IGN6_COUNTER, IGN6_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if IGN_CHANNELS >= 7
IgnitionSchedule ignitionSchedule7(IGN7_COUNTER, IGN7_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if IGN_CHANNELS >= 8
IgnitionSchedule ignitionSchedule8(IGN8_COUNTER, IGN8_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif

Schedule::Schedule(counter_t &_counter, compare_t &_compare)
  : _duration(0U)
  , _status(OFF)
  , _pStartCallback(nullCallback)
  , _pEndCallback(nullCallback)
  , _nextStartCompare(0U)
  , _counter(_counter)
  , _compare(_compare) 
{
}

static void reset(Schedule &schedule)
{
    schedule._status = OFF;
    setCallbacks(schedule, nullCallback, nullCallback);
}

static void reset(FuelSchedule &schedule) 
{
    reset((Schedule&)schedule);
    schedule.channelDegrees = 0;
}

static void reset(IgnitionSchedule &schedule) 
{
    reset((Schedule&)schedule);
    schedule.chargeAngle = 0;
    schedule.dischargeAngle = 0;
    schedule.channelDegrees = 0;
}

void initialiseSchedulers(void)
{
    reset(fuelSchedule1);
    reset(fuelSchedule2);
    reset(fuelSchedule3);
    reset(fuelSchedule4);
#if INJ_CHANNELS >= 5
    reset(fuelSchedule5);
#endif
#if INJ_CHANNELS >= 6
    reset(fuelSchedule6);
#endif
#if INJ_CHANNELS >= 7
    reset(fuelSchedule7);
#endif
#if INJ_CHANNELS >= 8
    reset(fuelSchedule8);
#endif

    reset(ignitionSchedule1);
    reset(ignitionSchedule2);
    reset(ignitionSchedule3);
    reset(ignitionSchedule4);
#if (IGN_CHANNELS >= 5)
    reset(ignitionSchedule5);
#endif
#if IGN_CHANNELS >= 6
    reset(ignitionSchedule6);
#endif
#if IGN_CHANNELS >= 7
    reset(ignitionSchedule7);
#endif
#if IGN_CHANNELS >= 8
    reset(ignitionSchedule8);
#endif
}

void startSchedulers(void)
{
  FUEL1_TIMER_ENABLE();
  FUEL2_TIMER_ENABLE();
  FUEL3_TIMER_ENABLE();
  FUEL4_TIMER_ENABLE();
#if INJ_CHANNELS >= 5
  FUEL5_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 6
  FUEL6_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 7
  FUEL7_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 8
  FUEL8_TIMER_ENABLE();
#endif

  IGN1_TIMER_ENABLE();
  IGN2_TIMER_ENABLE();
  IGN3_TIMER_ENABLE();
  IGN4_TIMER_ENABLE();
  IGN5_TIMER_ENABLE();
#if IGN_CHANNELS >= 6
  IGN6_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 7
  IGN7_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 8
  IGN8_TIMER_ENABLE();
#endif  
}

void setCallbacks(Schedule &schedule, voidVoidCallback pStartCallback, voidVoidCallback pEndCallback)
{
  schedule._pStartCallback = pStartCallback;
  schedule._pEndCallback = pEndCallback;
}

void _setSchedulePending(Schedule &schedule, uint32_t timeout, uint32_t duration)
{
  //The following must be enclosed in the noInterupts block to avoid contention caused if the relevant interrupt fires before the state is fully set
  schedule._duration = uS_TO_TIMER_COMPARE(duration);
  schedule._compare = schedule._counter + uS_TO_TIMER_COMPARE(timeout);
  schedule._status = PENDING; //Turn this schedule on
}

void _setScheduleNext(Schedule &schedule, uint32_t timeout, uint32_t duration)
{
  //If the schedule is already running, we can set the next schedule so it is ready to go
  //This is required in cases of high rpm and high DC where there otherwise would not be enough time to set the schedule
  schedule._nextStartCompare = schedule._counter + uS_TO_TIMER_COMPARE(timeout);
  // Schedule must already be running, so safe to reuse this.
  schedule._duration = uS_TO_TIMER_COMPARE(duration);
  schedule._status = RUNNING_WITHNEXT;
}

static inline void applyChannelOverDwellProtection(IgnitionSchedule &schedule, uint32_t targetOverdwellTime) {
  ATOMIC() {  
    if (isRunning(schedule)) {
      if (schedule._startTime < targetOverdwellTime) {
        schedule._pEndCallback(); 
        schedule._status = OFF;     
      }
    }
  }
}

void applyOverDwellProtection(void)
{
  bool isCrankLocked = configPage4.ignCranklock && (currentStatus.RPM < currentStatus.crankRPM); //Dwell limiter is disabled during cranking on setups using the locked cranking timing. WE HAVE to do the RPM check here as relying on the engine cranking bit can be potentially too slow in updating
  if ((configPage4.useDwellLim) && (isCrankLocked != true)) {
    uint32_t targetOverdwellTime = micros() - configPage4.dwellLimit * 1000U;
    
    applyChannelOverDwellProtection(ignitionSchedule1, targetOverdwellTime);
#if IGN_CHANNELS >= 2
    applyChannelOverDwellProtection(ignitionSchedule2, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 3
    applyChannelOverDwellProtection(ignitionSchedule3, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 4
    applyChannelOverDwellProtection(ignitionSchedule4, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 5
    applyChannelOverDwellProtection(ignitionSchedule5, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 6
    applyChannelOverDwellProtection(ignitionSchedule6, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 7
    applyChannelOverDwellProtection(ignitionSchedule7, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 8
    applyChannelOverDwellProtection(ignitionSchedule8, targetOverdwellTime);
#endif
  }
}

/** Perform the injector priming pulses.
 * Set these to run at an arbitrary time in the future (100us).
 * The prime pulse value is in ms*10, so need to multiple by 100 to get to uS
 */
void beginInjectorPriming(void)
{
  uint32_t primingValue = (uint32_t)table2D_getValue(&PrimingPulseTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
  if( (primingValue > 0U) && (currentStatus.TPS < configPage4.floodClear) )
  {
    primingValue = primingValue * 100UL * 5UL; //to achieve long enough priming pulses, the values in tuner studio are divided by 0.5 instead of 0.1, so multiplier of 5 is required.
    if ( maxInjOutputs >= 1U ) { setFuelSchedule(fuelSchedule1, 100U, primingValue); }
#if (INJ_CHANNELS >= 2)
    if ( maxInjOutputs >= 2U ) { setFuelSchedule(fuelSchedule2, 100U, primingValue); }
#endif
#if (INJ_CHANNELS >= 3)
    if ( maxInjOutputs >= 3U ) { setFuelSchedule(fuelSchedule3, 100U, primingValue); }
#endif
#if (INJ_CHANNELS >= 4)
    if ( maxInjOutputs >= 4U ) { setFuelSchedule(fuelSchedule4, 100U, primingValue); }
#endif
#if (INJ_CHANNELS >= 5)
    if ( maxInjOutputs >= 5U ) { setFuelSchedule(fuelSchedule5, 100U, primingValue); }
#endif
#if (INJ_CHANNELS >= 6)
    if ( maxInjOutputs >= 6U ) { setFuelSchedule(fuelSchedule6, 100U, primingValue); }
#endif
#if (INJ_CHANNELS >= 7)
    if ( maxInjOutputs >= 7U) { setFuelSchedule(fuelSchedule7, 100U, primingValue); }
#endif
#if (INJ_CHANNELS >= 8)
    if ( maxInjOutputs >= 8U ) { setFuelSchedule(fuelSchedule8, 100U, primingValue); }
#endif
  }
}

/**
 * @defgroup fuel-schedule-ISR Fuel schedule timer ISRs 
 *   
 * @{
 */

void moveToNextState(FuelSchedule &schedule)
{
  movetoNextState(schedule, defaultPendingToRunning, defaultRunningToOff, defaultRunningToPending);
} 

///@}

/**
 * @defgroup ignition-schedule-ISR Ignition schedule timer ISRs 
 *   
 * @{
 */

///@cond
// Dwell smoothing macros. They are split up like this for MISRA compliance.
#define DWELL_AVERAGE_ALPHA 30
#define DWELL_AVERAGE(input) LOW_PASS_FILTER((input), DWELL_AVERAGE_ALPHA, currentStatus.actualDwell)
//#define DWELL_AVERAGE(input) (currentStatus.dwell) //Can be use to disable the above for testing
///@endcond

/**
 * @brief Called when an ignition event ends. I.e. a spark fires
 * 
 * @param pSchedule Pointer to the schedule that fired the spark
 */
static inline void onEndIgnitionEvent(IgnitionSchedule *pSchedule) {
  ignitionCount = ignitionCount + 1U; //Increment the ignition counter
  int32_t elapsed = (int32_t)(micros() - pSchedule->_startTime);
  currentStatus.actualDwell = (uint16_t)DWELL_AVERAGE( elapsed );
}

/** @brief Called when the supplied schedule transitions from a PENDING state to RUNNING */
static inline void ignitionPendingToRunning(Schedule *pSchedule) {
  defaultPendingToRunning(pSchedule);

  // cppcheck-suppress misra-c2012-11.3 ; A cast from pointer to base to pointer to derived must point to the same location
  IgnitionSchedule *pIgnition = (IgnitionSchedule *)pSchedule;
  pIgnition->_startTime = micros();
}

/** @brief Called when the supplied schedule transitions from a RUNNING state to OFF */
static inline void ignitionRunningToOff(Schedule *pSchedule) {
  defaultRunningToOff(pSchedule);
  // cppcheck-suppress misra-c2012-11.3 ; A cast from pointer to base to pointer to derived must point to the same location
  onEndIgnitionEvent((IgnitionSchedule *)pSchedule);
}

/** @brief Called when the supplied schedule transitions from a RUNNING state to PENDING */
static inline void ignitionRunningToPending(Schedule *pSchedule) {
  defaultRunningToPending(pSchedule);
  // cppcheck-suppress misra-c2012-11.3 ; A cast from pointer to base to pointer to derived must point to the same location
  onEndIgnitionEvent((IgnitionSchedule *)pSchedule);
}

void moveToNextState(IgnitionSchedule &schedule)
{
  movetoNextState(schedule, ignitionPendingToRunning, ignitionRunningToOff, ignitionRunningToPending);
}

///@}

static void disableScheduleIfPending(Schedule &schedule) {
  ATOMIC() {
    if(schedule._status != RUNNING) { schedule._status = OFF; }  
  }
}

void disablePendingFuelSchedule(byte channel)
{
  ATOMIC() {
    switch(channel)
    {
      case 0: disableScheduleIfPending(fuelSchedule1); break;
      case 1: disableScheduleIfPending(fuelSchedule2); break;
      case 2: disableScheduleIfPending(fuelSchedule3); break;
      case 3: disableScheduleIfPending(fuelSchedule4); break;
  #if (INJ_CHANNELS >= 5)
      case 4: disableScheduleIfPending(fuelSchedule5); break;
  #endif
  #if (INJ_CHANNELS >= 6)
      case 5: disableScheduleIfPending(fuelSchedule6); break;
  #endif
  #if (INJ_CHANNELS >= 7)
      case 6: disableScheduleIfPending(fuelSchedule7); break;
  #endif
  #if (INJ_CHANNELS >= 8)
      case 7: disableScheduleIfPending(fuelSchedule8); break;
  #endif
      default: break;
    }
  }
}
void disablePendingIgnSchedule(byte channel)
{
  ATOMIC() {
    switch(channel)
    {
      case 0: disableScheduleIfPending(ignitionSchedule1); break;
      case 1: disableScheduleIfPending(ignitionSchedule2); break;
      case 2: disableScheduleIfPending(ignitionSchedule3); break;
      case 3: disableScheduleIfPending(ignitionSchedule4); break;
  #if (IGN_CHANNELS >= 5)
      case 4: disableScheduleIfPending(ignitionSchedule5); break;
  #endif
  #if (IGN_CHANNELS >= 6)
      case 5: disableScheduleIfPending(ignitionSchedule6); break;
  #endif
  #if (IGN_CHANNELS >= 7)
      case 6: disableScheduleIfPending(ignitionSchedule7); break;
  #endif
  #if (IGN_CHANNELS >= 8)
      case 7: disableScheduleIfPending(ignitionSchedule8); break;
  #endif
      default:break;
    }
  }
}
