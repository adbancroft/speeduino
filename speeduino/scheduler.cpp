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
#include "schedule_calcs.h"
#include "utilities.h"
#include "units.h"
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

Schedule::Schedule(counter_t &counter, compare_t &compare)
  : Duration(0U)
  , Status(OFF)
  , pStartCallback(nullCallback)
  , pEndCallback(nullCallback)
  , nextStartCompare(0U)
  , _counter(counter)
  , _compare(compare) 
{
}

static void reset(Schedule &schedule)
{
    schedule.Status = OFF;
    setCallbacks(schedule, nullCallback, nullCallback);
}

static void reset(FuelSchedule &schedule) 
{
    reset((Schedule&)schedule);
}

static void reset(IgnitionSchedule &schedule) 
{
    reset((Schedule&)schedule);
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

  ignition1StartAngle=0;
  ignition1EndAngle=0;
  channel1IgnDegrees=0; /**< The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones) */

  ignition2StartAngle=0;
  ignition2EndAngle=0;
  channel2IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */

  ignition3StartAngle=0;
  ignition3EndAngle=0;
  channel3IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */

  ignition4StartAngle=0;
  ignition4EndAngle=0;
  channel4IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */

#if (IGN_CHANNELS >= 5)
  ignition5StartAngle=0;
  ignition5EndAngle=0;
  channel5IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
#endif
#if (IGN_CHANNELS >= 6)
  ignition6StartAngle=0;
  ignition6EndAngle=0;
  channel6IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
#endif
#if (IGN_CHANNELS >= 7)
  ignition7StartAngle=0;
  ignition7EndAngle=0;
  channel7IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
#endif
#if (IGN_CHANNELS >= 8)
  ignition8StartAngle=0;
  ignition8EndAngle=0;
  channel8IgnDegrees=0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
#endif

	channel1InjDegrees = 0; /**< The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones) */
	channel2InjDegrees = 0; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
	channel3InjDegrees = 0; /**< The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC */
	channel4InjDegrees = 0; /**< The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC */
#if (INJ_CHANNELS >= 5)
	channel5InjDegrees = 0; /**< The number of crank degrees until cylinder 5 is at TDC */
#endif
#if (INJ_CHANNELS >= 6)
	channel6InjDegrees = 0; /**< The number of crank degrees until cylinder 6 is at TDC */
#endif
#if (INJ_CHANNELS >= 7)
	channel7InjDegrees = 0; /**< The number of crank degrees until cylinder 7 is at TDC */
#endif
#if (INJ_CHANNELS >= 8)
	channel8InjDegrees = 0; /**< The number of crank degrees until cylinder 8 is at TDC */
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

static inline void clearNextSchedule(Schedule &schedule) {
  schedule.Status = RUNNING;
  schedule.Duration = 0U;
}

void setCallbacks(Schedule &schedule, voidVoidCallback pStartCallback, voidVoidCallback pEndCallback)
{
  schedule.pStartCallback = pStartCallback;
  schedule.pEndCallback = pEndCallback;
}

void refreshIgnitionSchedule1(unsigned long timeToEnd)
{
  if( isRunning(ignitionSchedule1) && (uS_TO_TIMER_COMPARE(timeToEnd) < ignitionSchedule1.Duration) )
  //Must have the threshold check here otherwise it can cause a condition where the compare fires twice, once after the other, both for the end
  //if( (timeToEnd < ignitionSchedule1.duration) && (timeToEnd > IGNITION_REFRESH_THRESHOLD) )
  {
    noInterrupts();
    ignitionSchedule1.Duration = uS_TO_TIMER_COMPARE(timeToEnd);
    ignitionSchedule1._compare = ignitionSchedule1._counter + ignitionSchedule1.Duration;
    interrupts();
  }
}

static table2D_u8_u8_4 PrimingPulseTable(&configPage2.primeBins, &configPage2.primePulse);

/** Perform the injector priming pulses.
 * Set these to run at an arbitrary time in the future (100us).
 * The prime pulse value is in ms*10, so need to multiple by 100 to get to uS
 */
extern void beginInjectorPriming(void)
{
  unsigned long primingValue = table2D_getValue(&PrimingPulseTable, temperatureAddOffset(currentStatus.coolant));
  if( (primingValue > 0U) && (currentStatus.TPS <= configPage4.floodClear) )
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

static void disableSchedule(Schedule &schedule)
{
  if(schedule.Status == PENDING) 
  { 
    schedule.Status = OFF; 
  }
  else if(schedule.Status == RUNNING) 
  { 
    clearNextSchedule(schedule); 
  }
}

void disableFuelSchedule(byte channel)
{
  noInterrupts();
  switch(channel)
  {
    case 0:
      disableSchedule(fuelSchedule1);
      break;
    case 1:
      disableSchedule(fuelSchedule2);
      break;
    case 2: 
      disableSchedule(fuelSchedule3);
      break;
    case 3:
      disableSchedule(fuelSchedule4);
      break;
    case 4:
#if (INJ_CHANNELS >= 5)
      disableSchedule(fuelSchedule5);
#endif
      break;
    case 5:
#if (INJ_CHANNELS >= 6)
      disableSchedule(fuelSchedule6);
#endif
      break;
    case 6:
#if (INJ_CHANNELS >= 7)
      disableSchedule(fuelSchedule7);
#endif
      break;
    case 7:
#if (INJ_CHANNELS >= 8)
      disableSchedule(fuelSchedule8);
#endif
      break;
    default: break;
  }
  interrupts();
}
void disableIgnSchedule(byte channel)
{
  noInterrupts();
  switch(channel)
  {
    case 0:
      disableSchedule(ignitionSchedule1);
      break;
    case 1:
      disableSchedule(ignitionSchedule2);
      break;
    case 2: 
      disableSchedule(ignitionSchedule3);
      break;
    case 3:
      disableSchedule(ignitionSchedule4);
      break;
    case 4:
      disableSchedule(ignitionSchedule5);
      break;
#if IGN_CHANNELS >= 6      
    case 5:
      disableSchedule(ignitionSchedule6);
      break;
#endif
#if IGN_CHANNELS >= 7      
    case 6:
      disableSchedule(ignitionSchedule7);
      break;
#endif
#if IGN_CHANNELS >= 8      
    case 7:
      disableSchedule(ignitionSchedule8);
      break;
#endif
    default:break;
  }
  interrupts();
}

void disableAllFuelSchedules()
{
  disableFuelSchedule(0);
  disableFuelSchedule(1);
  disableFuelSchedule(2);
  disableFuelSchedule(3);
  disableFuelSchedule(4);
  disableFuelSchedule(5);
  disableFuelSchedule(6);
  disableFuelSchedule(7);
}
void disableAllIgnSchedules()
{
  disableIgnSchedule(0);
  disableIgnSchedule(1);
  disableIgnSchedule(2);
  disableIgnSchedule(3);
  disableIgnSchedule(4);
  disableIgnSchedule(5);
  disableIgnSchedule(6);
  disableIgnSchedule(7);
}
