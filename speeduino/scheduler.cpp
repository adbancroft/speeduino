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
#include "unit_testing.h"

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

void resetFuelSchedulers(void)
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
}

void resetIgnitionSchedulers(void)
{
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

void initialiseSchedulers()
{
  resetFuelSchedulers();
  resetIgnitionSchedulers();
}

void startFuelSchedulers(void)
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
}

void startIgnitionSchedulers(void)
{
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


void startSchedulers(void)
{
  startFuelSchedulers();
  startIgnitionSchedulers();
}

void setCallbacks(Schedule &schedule, voidVoidCallback pStartCallback, voidVoidCallback pEndCallback)
{
  schedule._pStartCallback = pStartCallback;
  schedule._pEndCallback = pEndCallback;
}

/** Perform the injector priming pulses.
 * Set these to run at an arbitrary time in the future (100us).
 * The prime pulse value is in ms*10, so need to multiple by 100 to get to uS
 */
void beginInjectorPriming(void)
{
  uint32_t primingValue = (uint32_t)table2D_getValue(&PrimingPulseTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
  if( (primingValue > UINT32_C(0)) && (currentStatus.TPS <= configPage4.floodClear) )
  {
    primingValue = primingValue * 100U * 5U; //to achieve long enough priming pulses, the values in tuner studio are divided by 0.5 instead of 0.1, so multiplier of 5 is required.
    if ( maxInjOutputs >= 1U ) { setFuelSchedule(fuelSchedule1, 100, primingValue); }
#if (INJ_CHANNELS >= 2)
    if ( maxInjOutputs >= 2U ) { setFuelSchedule(fuelSchedule2, 100, primingValue); }
#endif
#if (INJ_CHANNELS >= 3)
    if ( maxInjOutputs >= 3U ) { setFuelSchedule(fuelSchedule3, 100, primingValue); }
#endif
#if (INJ_CHANNELS >= 4)
    if ( maxInjOutputs >= 4U ) { setFuelSchedule(fuelSchedule4, 100, primingValue); }
#endif
#if (INJ_CHANNELS >= 5)
    if ( maxInjOutputs >= 5U ) { setFuelSchedule(fuelSchedule5, 100, primingValue); }
#endif
#if (INJ_CHANNELS >= 6)
    if ( maxInjOutputs >= 6U ) { setFuelSchedule(fuelSchedule6, 100, primingValue); }
#endif
#if (INJ_CHANNELS >= 7)
    if ( maxInjOutputs >= 7U ) { setFuelSchedule(fuelSchedule7, 100, primingValue); }
#endif
#if (INJ_CHANNELS >= 8)
    if ( maxInjOutputs >= 8U ) { setFuelSchedule(fuelSchedule8, 100, primingValue); }
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

void disablePendingFuelSchedule(byte channel)
{
  noInterrupts();
  switch(channel)
  {
    case 0:
      if(fuelSchedule1._status == PENDING) { fuelSchedule1._status = OFF; }
      break;
    case 1:
      if(fuelSchedule2._status == PENDING) { fuelSchedule2._status = OFF; }
      break;
    case 2: 
      if(fuelSchedule3._status == PENDING) { fuelSchedule3._status = OFF; }
      break;
    case 3:
      if(fuelSchedule4._status == PENDING) { fuelSchedule4._status = OFF; }
      break;
    case 4:
#if (INJ_CHANNELS >= 5)
      if(fuelSchedule5._status == PENDING) { fuelSchedule5._status = OFF; }
#endif
      break;
    case 5:
#if (INJ_CHANNELS >= 6)
      if(fuelSchedule6._status == PENDING) { fuelSchedule6._status = OFF; }
#endif
      break;
    case 6:
#if (INJ_CHANNELS >= 7)
      if(fuelSchedule7._status == PENDING) { fuelSchedule7._status = OFF; }
#endif
      break;
    case 7:
#if (INJ_CHANNELS >= 8)
      if(fuelSchedule8._status == PENDING) { fuelSchedule8._status = OFF; }
#endif
      break;
    default: break;
  }
  interrupts();
}
void disablePendingIgnSchedule(byte channel)
{
  noInterrupts();
  switch(channel)
  {
    case 0:
      if(ignitionSchedule1._status == PENDING) { ignitionSchedule1._status = OFF; }
      break;
    case 1:
      if(ignitionSchedule2._status == PENDING) { ignitionSchedule2._status = OFF; }
      break;
    case 2: 
      if(ignitionSchedule3._status == PENDING) { ignitionSchedule3._status = OFF; }
      break;
    case 3:
      if(ignitionSchedule4._status == PENDING) { ignitionSchedule4._status = OFF; }
      break;
    case 4:
      if(ignitionSchedule5._status == PENDING) { ignitionSchedule5._status = OFF; }
      break;
#if IGN_CHANNELS >= 6      
    case 6:
      if(ignitionSchedule6._status == PENDING) { ignitionSchedule6._status = OFF; }
      break;
#endif
#if IGN_CHANNELS >= 7      
    case 7:
      if(ignitionSchedule7._status == PENDING) { ignitionSchedule7._status = OFF; }
      break;
#endif
#if IGN_CHANNELS >= 8      
    case 8:
      if(ignitionSchedule8._status == PENDING) { ignitionSchedule8._status = OFF; }
      break;
#endif
    default:break;
  }
  interrupts();
}

TESTABLE_INLINE_STATIC void applyChannelOverDwellProtection(IgnitionSchedule &schedule, uint32_t targetOverdwellTime) {
  //Check first whether each spark output is currently on. Only check it's dwell time if it is
  ATOMIC() {
    if (isRunning(schedule) && unlikely(schedule._startTime < targetOverdwellTime)) { 
      ignitionRunningToOff(&schedule); //Call the end function to disable the spark output
    }
  }
}

TESTABLE_INLINE_STATIC bool isOverDwellActive(const config4 &page4, const statuses &current){
  bool isCrankLocked = page4.ignCranklock && (current.RPM < current.crankRPM); //Dwell limiter is disabled during cranking on setups using the locked cranking timing. WE HAVE to do the RPM check here as relying on the engine cranking bit can be potentially too slow in updating
  return (page4.useDwellLim) && likely(!isCrankLocked);
}

void applyOverDwellProtection(void)
{
  if (likely(isOverDwellActive(configPage4, currentStatus))) {
    uint32_t targetOverdwellTime = micros() - (configPage4.dwellLimit * 1000U); //Convert to uS

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
