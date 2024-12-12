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
#include "speeduino.h"

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
TESTABLE_INLINE_STATIC void setCallbacks(Schedule &schedule, voidVoidCallback pStartCallback, voidVoidCallback pEndCallback)
{
  schedule._pStartCallback = pStartCallback;
  schedule._pEndCallback = pEndCallback;
}

static inline void reset(Schedule &schedule)
{
    schedule._status = OFF;
    setCallbacks(schedule, nullCallback, nullCallback);
}

static inline void reset(FuelSchedule &schedule) 
{
    reset((Schedule&)schedule);
    schedule.channelDegrees = 0;
}

static inline void reset(IgnitionSchedule &schedule) 
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


static void turnOffInjectors(void)
{
  closeInjector1();
  closeInjector2();
  closeInjector3();
  closeInjector4();
  closeInjector5();
  #if (INJ_CHANNELS >= 6)
  closeInjector6();
  #endif
  #if (INJ_CHANNELS >= 7)
  closeInjector7();
  #endif
  #if (INJ_CHANNELS >= 8)
  closeInjector8();
  #endif
}

static void setFuelChannelAngles(void)
{
  switch (configPage2.nCylinders) {
  case 1:
      fuelSchedule1.channelDegrees = 0;
      maxInjOutputs = 1;

      //Check if injector staging is enabled
      if(configPage10.stagingEnabled == true)
      {
        maxInjOutputs = 2;
        fuelSchedule2.channelDegrees = fuelSchedule1.channelDegrees;
      }
      break;

  case 2:
      fuelSchedule1.channelDegrees = 0;
      maxInjOutputs = 2;
      //The below are true regardless of whether this is running sequential or not
      if (configPage2.engineType == EVEN_FIRE ) { fuelSchedule2.channelDegrees = CRANK_ANGLE_MAX_INJ / 2U; }
      else { fuelSchedule2.channelDegrees = configPage2.oddfire2; }

      if (!configPage2.injTiming) 
      { 
        //For simultaneous, all squirts happen at the same time
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 0; 
      }

      //Check if injector staging is enabled
      if(configPage10.stagingEnabled == true)
      {
        maxInjOutputs = 4;

        fuelSchedule3.channelDegrees = fuelSchedule1.channelDegrees;
        //Phase this either 180 or 360 degrees out from inj3 (In reality this will always be 180 as you can't have sequential and staged currently)
        fuelSchedule4.channelDegrees = fuelSchedule3.channelDegrees + (uint16_t)(CRANK_ANGLE_MAX_INJ / 2U); 
        if (fuelSchedule4.channelDegrees>=(uint16_t)CRANK_ANGLE_MAX_INJ) { fuelSchedule4.channelDegrees -= (uint16_t)CRANK_ANGLE_MAX_INJ; }
      }
      break;

  case 3:
        //For alternating injection, the squirt occurs at different times for each channel
      if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) )
      {
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 120;
        fuelSchedule3.channelDegrees = 240;

        if(configPage2.injType == INJ_TYPE_PORT)
        { 
          //Force nSquirts to 2 for individual port injection. This prevents TunerStudio forcing the value to 3 even when this isn't wanted. 
          currentStatus.nSquirts = 2;
          if(configPage2.strokes == FOUR_STROKE) { CRANK_ANGLE_MAX_INJ = 360; }
          else { CRANK_ANGLE_MAX_INJ = 180; }
        }        

        //Adjust the injection angles based on the number of squirts
        if (currentStatus.nSquirts > 2U)
        {
          fuelSchedule2.channelDegrees = (fuelSchedule2.channelDegrees * 2U) / currentStatus.nSquirts;
          fuelSchedule3.channelDegrees = (fuelSchedule3.channelDegrees * 2U) / currentStatus.nSquirts;
        }

        if (!configPage2.injTiming) 
        { 
          //For simultaneous, all squirts happen at the same time
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 0;
          fuelSchedule3.channelDegrees = 0; 
        } 
      }
      else if (configPage2.injLayout == INJ_SEQUENTIAL)
      {
        if(configPage2.strokes == TWO_STROKE)
        {
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 120;
          fuelSchedule3.channelDegrees = 240;
        }
        else
        {
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 240;
          fuelSchedule3.channelDegrees = 480;
        }
      }
      else
      {
        //Should never happen, but default values
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 120;
        fuelSchedule3.channelDegrees = 240;
      }

      maxInjOutputs = 3;

      //Check if injector staging is enabled
      if(configPage10.stagingEnabled == true)
      {
        #if INJ_CHANNELS >= 6
          maxInjOutputs = 6;

          fuelSchedule4.channelDegrees = fuelSchedule1.channelDegrees;
          fuelSchedule5.channelDegrees = fuelSchedule2.channelDegrees;
          fuelSchedule6.channelDegrees = fuelSchedule3.channelDegrees;
        #else
          //Staged output is on channel 4
          maxInjOutputs = 4;
          fuelSchedule4.channelDegrees = fuelSchedule1.channelDegrees;
        #endif
      }
      break;

  case 4:
      fuelSchedule1.channelDegrees = 0;
      maxInjOutputs = 2;

      //For alternating injection, the squirt occurs at different times for each channel
      if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) || (configPage2.strokes == TWO_STROKE) )
      {
        fuelSchedule2.channelDegrees = 180;

        if (!configPage2.injTiming) 
        { 
          //For simultaneous, all squirts happen at the same time
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 0; 
        }
        else if (currentStatus.nSquirts > 2U)
        {
          //Adjust the injection angles based on the number of squirts
          fuelSchedule2.channelDegrees = (fuelSchedule2.channelDegrees * 2U) / currentStatus.nSquirts;
        }
        else { } //Do nothing, default values are correct
      }
      else if (configPage2.injLayout == INJ_SEQUENTIAL)
      {
        fuelSchedule2.channelDegrees = 180;
        fuelSchedule3.channelDegrees = 360;
        fuelSchedule4.channelDegrees = 540;
        
        maxInjOutputs = 4;
      }
      else
      {
        //Should never happen, but default values
      }

      //Check if injector staging is enabled
      if(configPage10.stagingEnabled == true)
      {
        maxInjOutputs = 4;

        if( (configPage2.injLayout == INJ_SEQUENTIAL) || (configPage2.injLayout == INJ_SEMISEQUENTIAL) )
        {
          //Staging with 4 cylinders semi/sequential requires 8 total channels
          #if INJ_CHANNELS >= 8
          maxInjOutputs = 8;
          fuelSchedule5.channelDegrees = fuelSchedule1.channelDegrees;
          fuelSchedule6.channelDegrees = fuelSchedule2.channelDegrees;
          fuelSchedule7.channelDegrees = fuelSchedule3.channelDegrees;
          fuelSchedule8.channelDegrees = fuelSchedule4.channelDegrees;
          #elif INJ_CHANNELS >= 5
          //This is an invalid config as there are not enough outputs to support sequential + staging
          //Put the staging output to the non-existant channel 5
          maxInjOutputs = 5;
          fuelSchedule5.channelDegrees = fuelSchedule1.channelDegrees;
          #endif
        }
        else
        {
          fuelSchedule3.channelDegrees = fuelSchedule1.channelDegrees;
          fuelSchedule4.channelDegrees = fuelSchedule2.channelDegrees;
        }
      }
      break;

  case 5:
      maxInjOutputs = 4;
      //For alternating injection, the squirt occurs at different times for each channel
      if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) || (configPage2.strokes == TWO_STROKE) )
      {
        if (!configPage2.injTiming) 
        { 
          //For simultaneous, all squirts happen at the same time
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 0;
          fuelSchedule3.channelDegrees = 0;
          fuelSchedule4.channelDegrees = 0;
#if (INJ_CHANNELS >= 5)
          fuelSchedule5.channelDegrees = 0; 
          maxInjOutputs = 5;
#endif
        }
        else
        {
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 72;
          fuelSchedule3.channelDegrees = 144;
          fuelSchedule4.channelDegrees = 216;
#if (INJ_CHANNELS >= 5)
          fuelSchedule5.channelDegrees = 288;
          maxInjOutputs = 5;
#endif

          //Divide by currentStatus.nSquirts ?
        }
      }
  #if INJ_CHANNELS >= 5
      else if (configPage2.injLayout == INJ_SEQUENTIAL)
      {
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 144;
        fuelSchedule3.channelDegrees = 288;
        fuelSchedule4.channelDegrees = 432;
        fuelSchedule5.channelDegrees = 576;
        maxInjOutputs = 5;
      }
  #endif
#if INJ_CHANNELS >= 6
          if(configPage10.stagingEnabled == true) { maxInjOutputs = 6; }
#endif
      break;

  case 6:
      maxInjOutputs = 3;

      //For alternating injection, the squirt occurs at different times for each channel
      if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) )
      {
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 120;
        fuelSchedule3.channelDegrees = 240;
        if (!configPage2.injTiming)
        {
          //For simultaneous, all squirts happen at the same time
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 0;
          fuelSchedule3.channelDegrees = 0;
        }
        else if (currentStatus.nSquirts > 2U)
        {
          //Adjust the injection angles based on the number of squirts
          fuelSchedule2.channelDegrees = (fuelSchedule2.channelDegrees * 2U) / currentStatus.nSquirts;
          fuelSchedule3.channelDegrees = (fuelSchedule3.channelDegrees * 2U) / currentStatus.nSquirts;
        } else {
          // Nothing to do: keep MISRA checker happy
        }
      }

  #if INJ_CHANNELS >= 6
      if (configPage2.injLayout == INJ_SEQUENTIAL)
      {
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 120;
        fuelSchedule3.channelDegrees = 240;
        fuelSchedule4.channelDegrees = 360;
        fuelSchedule5.channelDegrees = 480;
        fuelSchedule6.channelDegrees = 600;

        maxInjOutputs = 6;
      }
      else if(configPage10.stagingEnabled == true) //Check if injector staging is enabled
      {
        maxInjOutputs = 6;

        if( (configPage2.injLayout == INJ_SEQUENTIAL) || (configPage2.injLayout == INJ_SEMISEQUENTIAL) )
        {
          //Staging with 6 cylinders semi/sequential requires 7 total channels
          #if INJ_CHANNELS >= 7
            maxInjOutputs = 7;

            fuelSchedule5.channelDegrees = fuelSchedule1.channelDegrees;
            fuelSchedule6.channelDegrees = fuelSchedule2.channelDegrees;
            fuelSchedule7.channelDegrees = fuelSchedule3.channelDegrees;
            fuelSchedule8.channelDegrees = fuelSchedule4.channelDegrees;
          #else
            //This is an invalid config as there are not enough outputs to support sequential + staging
            //Put the staging output to the non-existant channel 7
            maxInjOutputs = 6;
          #endif
        }
      }
  #endif
      break;

  case 8:
      maxInjOutputs = 4;
      //For alternating injection, the squirt occurs at different times for each channel
      if( (configPage2.injLayout == INJ_SEMISEQUENTIAL) || (configPage2.injLayout == INJ_PAIRED) )
      {
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 90;
        fuelSchedule3.channelDegrees = 180;
        fuelSchedule4.channelDegrees = 270;

        if (!configPage2.injTiming)
        {
          //For simultaneous, all squirts happen at the same time
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 0;
          fuelSchedule3.channelDegrees = 0;
          fuelSchedule4.channelDegrees = 0;
        }
        else if (currentStatus.nSquirts > 2U)
        {
          //Adjust the injection angles based on the number of squirts
          fuelSchedule2.channelDegrees = (fuelSchedule2.channelDegrees * 2U) / currentStatus.nSquirts;
          fuelSchedule3.channelDegrees = (fuelSchedule3.channelDegrees * 2U) / currentStatus.nSquirts;
          fuelSchedule4.channelDegrees = (fuelSchedule4.channelDegrees * 2U) / currentStatus.nSquirts;
        } else {
          // Keep MISRA checker happy.
        }
      }

  #if INJ_CHANNELS >= 8
      else if (configPage2.injLayout == INJ_SEQUENTIAL)
      {
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 90;
        fuelSchedule3.channelDegrees = 180;
        fuelSchedule4.channelDegrees = 270;
        fuelSchedule5.channelDegrees = 360;
        fuelSchedule6.channelDegrees = 450;
        fuelSchedule7.channelDegrees = 540;
        fuelSchedule8.channelDegrees = 630;
        maxInjOutputs = 8;
      }
  #endif
      break;
  default: //Handle this better!!!
      fuelSchedule1.channelDegrees = 0;
      fuelSchedule2.channelDegrees = 180;
      maxInjOutputs = 2;
      break;
  }  

  //Special case:
  //3 or 5 squirts per cycle MUST be tracked over 720 degrees. This is because the angles for them (Eg 720/3=240) are not evenly divisible into 360
  //This is ONLY the case on 4 stroke systems
  if( (currentStatus.nSquirts == 3U) || (currentStatus.nSquirts == 5U) )
  {
    if(configPage2.strokes == FOUR_STROKE) { CRANK_ANGLE_MAX_INJ = 720U/currentStatus.nSquirts; }
  }  
}

static void setFuelScheduleCallbacks(void)
{
  switch(configPage2.injLayout)
  {
  case INJ_PAIRED:
      //Paired injection
      setCallbacks(fuelSchedule1, openInjector1, closeInjector1);
      setCallbacks(fuelSchedule2, openInjector2, closeInjector2);
      setCallbacks(fuelSchedule3, openInjector3, closeInjector3);
      setCallbacks(fuelSchedule4, openInjector4, closeInjector4);
  #if INJ_CHANNELS >= 5
      setCallbacks(fuelSchedule5, openInjector5, closeInjector5);
  #endif
      break;

  case INJ_SEMISEQUENTIAL:
      //Semi-Sequential injection. Currently possible with 4, 6 and 8 cylinders. 5 cylinder is a special case
      if( configPage2.nCylinders == 4U )
      {
        if(configPage4.inj4cylPairing == INJ_PAIR_13_24)
        {
          setCallbacks(fuelSchedule1, openInjector1and3, closeInjector1and3);
          setCallbacks(fuelSchedule2, openInjector2and4, closeInjector2and4);
        }
        else
        {
          setCallbacks(fuelSchedule1, openInjector1and4, closeInjector1and4);
          setCallbacks(fuelSchedule2, openInjector2and3, closeInjector2and3);
        }
      }
      else if( configPage2.nCylinders == 5U ) //This is similar to the paired injection but uses five injector outputs instead of four
      {
        setCallbacks(fuelSchedule1, openInjector1, closeInjector1);
        setCallbacks(fuelSchedule2, openInjector2, closeInjector2);
        setCallbacks(fuelSchedule3, openInjector3and5, closeInjector3and5);
        setCallbacks(fuelSchedule4, openInjector4, closeInjector4);
      }
      else if( configPage2.nCylinders == 6U )
      {
        setCallbacks(fuelSchedule1, openInjector1and4, closeInjector1and4);
        setCallbacks(fuelSchedule2, openInjector2and5, closeInjector2and5);
        setCallbacks(fuelSchedule3, openInjector3and6, closeInjector3and6);
      }
      else if( configPage2.nCylinders == 8U )
      {
        setCallbacks(fuelSchedule1, openInjector1and5, closeInjector1and5);
        setCallbacks(fuelSchedule2, openInjector2and6, closeInjector2and6);
        setCallbacks(fuelSchedule3, openInjector3and7, closeInjector3and7);
        setCallbacks(fuelSchedule4, openInjector4and8, closeInjector4and8);
      }
      else
      {
        //Fall back to paired injection
        setCallbacks(fuelSchedule1, openInjector1, closeInjector1);
        setCallbacks(fuelSchedule2, openInjector2, closeInjector2);
        setCallbacks(fuelSchedule3, openInjector3, closeInjector3);
        setCallbacks(fuelSchedule4, openInjector4, closeInjector4);
  #if INJ_CHANNELS >= 5
        setCallbacks(fuelSchedule5, openInjector5, closeInjector5);
  #endif
      }
      break;

  case INJ_SEQUENTIAL:
      //Sequential injection
      setCallbacks(fuelSchedule1, openInjector1, closeInjector1);
      setCallbacks(fuelSchedule2, openInjector2, closeInjector2);
      setCallbacks(fuelSchedule3, openInjector3, closeInjector3);
      setCallbacks(fuelSchedule4, openInjector4, closeInjector4);
  #if INJ_CHANNELS >= 5
      setCallbacks(fuelSchedule5, openInjector5, closeInjector5);
  #endif
  #if INJ_CHANNELS >= 6
      setCallbacks(fuelSchedule6, openInjector6, closeInjector6);
  #endif
  #if INJ_CHANNELS >= 7
      setCallbacks(fuelSchedule7, openInjector7, closeInjector7);
  #endif
  #if INJ_CHANNELS >= 8
      setCallbacks(fuelSchedule8, openInjector8, closeInjector8);
  #endif
      break;

  default:
      //Paired injection
      setCallbacks(fuelSchedule1, openInjector1, closeInjector1);
      setCallbacks(fuelSchedule2, openInjector2, closeInjector2);
      setCallbacks(fuelSchedule3, openInjector3, closeInjector3);
      setCallbacks(fuelSchedule4, openInjector4, closeInjector4);
  #if INJ_CHANNELS >= 5
      setCallbacks(fuelSchedule5, openInjector5, closeInjector5);
  #endif
      break;
  }
}

static void initialiseFuelContext(void) 
{
  if(configPage2.strokes == FOUR_STROKE)
  {
    //Default is 1 squirt per revolution, so we halve the given req-fuel figure (Which would be over 2 revolutions)
    req_fuel_uS = req_fuel_uS / 2U; //The req_fuel calculation above gives the total required fuel (At VE 100%) in the full cycle. If we're doing more than 1 squirt per cycle then we need to split the amount accordingly. (Note that in a non-sequential 4-stroke setup you cannot have less than 2 squirts as you cannot determine the stroke to make the single squirt on)
  }

  if ( (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) && (configPage2.nCylinders<=(uint8_t)INJ_CHANNELS))
  {
    CRANK_ANGLE_MAX_INJ = 720;
    currentStatus.nSquirts = 1;
    req_fuel_uS = req_fuel_uS * 2U;
  }
  else
  {
    currentStatus.nSquirts = max(1, configPage2.nCylinders / configPage2.divider); //The number of squirts being requested. This is manually overridden below for sequential setups (Due to TS req_fuel calc limitations)
    if ((configPage2.injLayout == INJ_SEQUENTIAL) && configPage2.nCylinders==3U) {
      currentStatus.nSquirts = 1;
    }
    CRANK_ANGLE_MAX_INJ = (configPage2.strokes == FOUR_STROKE ? 720U : 360U) / currentStatus.nSquirts;
  }
  currentStatus.status3 |= currentStatus.nSquirts << BIT_STATUS3_NSQUIRTS1; //Top 3 bits of the status3 variable are the number of squirts. This must be done after the above section due to nSquirts being forced to 1 for sequential
}

void initialiseFuelSchedulers(void)
{
  resetFuelSchedulers();
  turnOffInjectors();
  initialiseFuelContext();
  setFuelChannelAngles();
  setFuelScheduleCallbacks();
  startFuelSchedulers();
}

void startIgnitionSchedulers(void)
{
  IGN1_TIMER_ENABLE();
  IGN2_TIMER_ENABLE();
  IGN3_TIMER_ENABLE();
  IGN4_TIMER_ENABLE();
#if IGN_CHANNELS >= 5
  IGN5_TIMER_ENABLE();
#endif
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


static void turnOffCoils(void)
{
  //End all coil charges to ensure no stray sparks on startup
  endCoil1Charge();
  endCoil2Charge();
  endCoil3Charge();
  endCoil4Charge();
  endCoil5Charge();
  #if (IGN_CHANNELS >= 6)
  endCoil6Charge();
  #endif
  #if (IGN_CHANNELS >= 7)
  endCoil7Charge();
  #endif
  #if (IGN_CHANNELS >= 8)
  endCoil8Charge();
  #endif
}

static void setIgnitionChannelAngles(void)
{
  switch (configPage2.nCylinders) {
  case 1:
      ignitionSchedule1.channelDegrees = 0;
      maxIgnOutputs = 1;
      break;

  case 2:
      ignitionSchedule1.channelDegrees = 0;
      maxIgnOutputs = 2;
      if (configPage2.engineType == EVEN_FIRE ) { ignitionSchedule2.channelDegrees = 180; }
      else { ignitionSchedule2.channelDegrees = configPage2.oddfire2; }
      break;

  case 3:
      ignitionSchedule1.channelDegrees = 0;
      maxIgnOutputs = 3;
      if (configPage2.engineType == EVEN_FIRE )
      {
        //Sequential and Single channel modes both run over 720 crank degrees, but only on 4 stroke engines.
        if( ( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) || (configPage4.sparkMode == IGN_MODE_SINGLE) ) && (configPage2.strokes == FOUR_STROKE) )
        {
          ignitionSchedule2.channelDegrees = 240;
          ignitionSchedule3.channelDegrees = 480;
        }
        else
        {
          ignitionSchedule2.channelDegrees = 120;
          ignitionSchedule3.channelDegrees = 240;
        }
      }
      else
      {
        ignitionSchedule2.channelDegrees = configPage2.oddfire2;
        ignitionSchedule3.channelDegrees = configPage2.oddfire3;
      }
      break;

  case 4:
      ignitionSchedule1.channelDegrees = 0;
      maxIgnOutputs = 2; //Default value for 4 cylinder, may be changed below
      if (configPage2.engineType == EVEN_FIRE )
      {
        ignitionSchedule2.channelDegrees = 180;

        if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) )
        {
          ignitionSchedule3.channelDegrees = 360;
          ignitionSchedule4.channelDegrees = 540;
          maxIgnOutputs = 4;
        }
        if(configPage4.sparkMode == IGN_MODE_ROTARY)
        {
          //Rotary uses the ign 3 and 4 schedules for the trailing spark. They are offset from the ign 1 and 2 channels respectively and so use the same degrees as them
          ignitionSchedule3.channelDegrees = 0;
          ignitionSchedule4.channelDegrees = 180;
          maxIgnOutputs = 4;

          configPage4.IgInv = GOING_LOW; //Force Going Low ignition mode (Going high is never used for rotary)
        }
      }
      else
      {
        ignitionSchedule2.channelDegrees = configPage2.oddfire2;
        ignitionSchedule3.channelDegrees = configPage2.oddfire3;
        ignitionSchedule4.channelDegrees = configPage2.oddfire4;
        maxIgnOutputs = 4;
      }
      break;

  case 5:
      ignitionSchedule1.channelDegrees = 0;
      ignitionSchedule2.channelDegrees = 72;
      ignitionSchedule3.channelDegrees = 144;
      ignitionSchedule4.channelDegrees = 216;
#if (IGN_CHANNELS >= 5)
      ignitionSchedule5.channelDegrees = 288;
      maxIgnOutputs = 5; //Only 4 actual outputs, so that's all that can be cut
#else
      maxIgnOutputs = 4; //Only 4 actual outputs, so that's all that can be cut
#endif

      if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
      {
        ignitionSchedule2.channelDegrees = 144;
        ignitionSchedule3.channelDegrees = 288;
        ignitionSchedule4.channelDegrees = 432;
#if (IGN_CHANNELS >= 5)
          ignitionSchedule5.channelDegrees = 576;
#endif
      }
      break;

  case 6:
      ignitionSchedule1.channelDegrees = 0;
      ignitionSchedule2.channelDegrees = 120;
      ignitionSchedule3.channelDegrees = 240;
      maxIgnOutputs = 3;

  #if IGN_CHANNELS >= 6
      if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL))
      {
        ignitionSchedule4.channelDegrees = 360;
        ignitionSchedule5.channelDegrees = 480;
        ignitionSchedule6.channelDegrees = 600;
        maxIgnOutputs = 6;
      }
  #endif
      break;

  case 8:
      ignitionSchedule1.channelDegrees = 0;
      ignitionSchedule2.channelDegrees = 90;
      ignitionSchedule3.channelDegrees = 180;
      ignitionSchedule4.channelDegrees = 270;
      maxIgnOutputs = 4;

      if(configPage4.sparkMode == IGN_MODE_SINGLE)
      {
        maxIgnOutputs = 4;
      }
  
  #if IGN_CHANNELS >= 8
      if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL))
      {
        ignitionSchedule5.channelDegrees = 360;
        ignitionSchedule6.channelDegrees = 450;
        ignitionSchedule7.channelDegrees = 540;
        ignitionSchedule8.channelDegrees = 630;
        maxIgnOutputs = 8;
      }
  #endif
      break;

  default:
      break;
  }
}

static void setIgnitionScheduleCallbacks(void)
{
  switch(configPage4.sparkMode)
  {
  case IGN_MODE_WASTED:
      //Wasted Spark (Normal mode)
      setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
      setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
      setCallbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge);
      setCallbacks(ignitionSchedule4, beginCoil4Charge, endCoil4Charge);
#if IGN_CHANNELS >= 5
      setCallbacks(ignitionSchedule5, beginCoil5Charge, endCoil5Charge);
#endif
      break;

  case IGN_MODE_SINGLE:
      //Single channel mode. All ignition pulses are on channel 1
      setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
      setCallbacks(ignitionSchedule2, beginCoil1Charge, endCoil1Charge);
      setCallbacks(ignitionSchedule3, beginCoil1Charge, endCoil1Charge);
      setCallbacks(ignitionSchedule4, beginCoil1Charge, endCoil1Charge);
#if IGN_CHANNELS >= 5
      setCallbacks(ignitionSchedule5, beginCoil1Charge, endCoil1Charge);
#endif
#if IGN_CHANNELS >= 6
      setCallbacks(ignitionSchedule6, beginCoil1Charge, endCoil1Charge);
#endif
#if IGN_CHANNELS >= 7
      setCallbacks(ignitionSchedule7, beginCoil1Charge, endCoil1Charge);
#endif
#if IGN_CHANNELS >= 8
      setCallbacks(ignitionSchedule8, beginCoil1Charge, endCoil1Charge);
#endif
      break;

  case IGN_MODE_WASTEDCOP:
      //Wasted COP mode. Note, most of the boards can only run this for 4-cyl only.
      if( configPage2.nCylinders <= 3U)
      {
          //1-3 cylinder wasted COP is the same as regular wasted mode
        setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
        setCallbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge);
        setCallbacks(ignitionSchedule4, beginCoil4Charge, endCoil4Charge);          
      }
      else if( configPage2.nCylinders == 4U )
      {
        //Wasted COP mode for 4 cylinders. Ignition channels 1&3 and 2&4 are paired together
        setCallbacks(ignitionSchedule1, beginCoil1and3Charge, endCoil1and3Charge);
        setCallbacks(ignitionSchedule2, beginCoil2and4Charge, endCoil2and4Charge);

        setCallbacks(ignitionSchedule3, nullCallback, nullCallback);
        setCallbacks(ignitionSchedule4, nullCallback, nullCallback);
      }
      else if( configPage2.nCylinders == 6U )
      {
        //Wasted COP mode for 6 cylinders. Ignition channels 1&4, 2&5 and 3&6 are paired together
        setCallbacks(ignitionSchedule1, beginCoil1and4Charge, endCoil1and4Charge);
        setCallbacks(ignitionSchedule2, beginCoil2and5Charge, endCoil2and5Charge);
        setCallbacks(ignitionSchedule3, beginCoil3and6Charge, endCoil3and6Charge);

        setCallbacks(ignitionSchedule4, nullCallback, nullCallback);
#if IGN_CHANNELS >= 5
        setCallbacks(ignitionSchedule5, nullCallback, nullCallback);
#endif
#if IGN_CHANNELS >= 6
        setCallbacks(ignitionSchedule6, nullCallback, nullCallback);
#endif
      }
      else if( configPage2.nCylinders == 8U )
      {
        //Wasted COP mode for 8 cylinders. Ignition channels 1&5, 2&6, 3&7 and 4&8 are paired together
        setCallbacks(ignitionSchedule1, beginCoil1and5Charge, endCoil1and5Charge);
        setCallbacks(ignitionSchedule2, beginCoil2and6Charge, endCoil2and6Charge);
        setCallbacks(ignitionSchedule3, beginCoil3and7Charge, endCoil3and7Charge);
        setCallbacks(ignitionSchedule4, beginCoil4and8Charge, endCoil4and8Charge);
#if IGN_CHANNELS >= 5
        setCallbacks(ignitionSchedule5, nullCallback, nullCallback);
#endif
#if IGN_CHANNELS >= 6
        setCallbacks(ignitionSchedule6, nullCallback, nullCallback);
#endif
#if IGN_CHANNELS >= 7
        setCallbacks(ignitionSchedule7, nullCallback, nullCallback);
#endif
#if IGN_CHANNELS >= 8
        setCallbacks(ignitionSchedule8, nullCallback, nullCallback);
#endif
      }
      else
      {
        //If the person has inadvertently selected this when running more than 4 cylinders or other than 6 cylinders, just use standard Wasted spark mode
        setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
        setCallbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge);
        setCallbacks(ignitionSchedule4, beginCoil4Charge, endCoil4Charge);
#if IGN_CHANNELS >= 5
        setCallbacks(ignitionSchedule5, beginCoil5Charge, endCoil5Charge);
#endif
      }
      break;

  case IGN_MODE_SEQUENTIAL:
      setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
      setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
      setCallbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge);
      setCallbacks(ignitionSchedule4, beginCoil4Charge, endCoil4Charge);
#if IGN_CHANNELS >= 5
      setCallbacks(ignitionSchedule5, beginCoil5Charge, endCoil5Charge);
#endif
#if IGN_CHANNELS >= 6
      setCallbacks(ignitionSchedule6, beginCoil6Charge, endCoil6Charge);
#endif
#if IGN_CHANNELS >= 7
      setCallbacks(ignitionSchedule7, beginCoil7Charge, endCoil7Charge);
#endif
#if IGN_CHANNELS >= 8
      setCallbacks(ignitionSchedule8, beginCoil8Charge, endCoil8Charge);
#endif
      break;

  case IGN_MODE_ROTARY:
      if(configPage10.rotaryType == ROTARY_IGN_FC)
      {
        //Ignition channel 1 is a wasted spark signal for leading signal on both rotors
        setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedule2, beginCoil1Charge, endCoil1Charge);

        setCallbacks(ignitionSchedule3, beginTrailingCoilCharge, endTrailingCoilCharge1);
        setCallbacks(ignitionSchedule4, beginTrailingCoilCharge, endTrailingCoilCharge2);
      }
      else if(configPage10.rotaryType == ROTARY_IGN_FD)
      {
        //Ignition channel 1 is a wasted spark signal for leading signal on both rotors
        setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedule2, beginCoil1Charge, endCoil1Charge);

        //Trailing coils have their own channel each
        //IGN2 = front rotor trailing spark
        setCallbacks(ignitionSchedule3, beginCoil2Charge, endCoil2Charge);
        //IGN3 = rear rotor trailing spark
        setCallbacks(ignitionSchedule4, beginCoil3Charge, endCoil3Charge);

        //IGN4 not used
      }
      else if(configPage10.rotaryType == ROTARY_IGN_RX8)
      {
        //RX8 outputs are simply 1 coil and 1 output per plug

        //IGN1 is front rotor, leading spark
        setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
        //IGN2 is rear rotor, leading spark
        setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
        //IGN3 = front rotor trailing spark
        setCallbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge);
        //IGN4 = rear rotor trailing spark
        setCallbacks(ignitionSchedule4, beginCoil4Charge, endCoil4Charge);
      }
      else { } //No action for other RX ignition modes (Future expansion / MISRA compliant). 
      break;

  default:
      //Wasted spark (Shouldn't ever happen anyway)
      setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
      setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
      setCallbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge);
      setCallbacks(ignitionSchedule4, beginCoil4Charge, endCoil4Charge);
#if IGN_CHANNELS >= 5
      setCallbacks(ignitionSchedule5, beginCoil5Charge, endCoil5Charge);
#endif
      break;
  }
}

static void initialiseIgnitionContext(void)
{
  CRANK_ANGLE_MAX_IGN = 360;
  if (configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
  {
    bool oddFireSequential = (configPage2.engineType == ODD_FIRE) && (configPage2.nCylinders<=2U);
    bool evenFireSequential = (configPage2.engineType == EVEN_FIRE) && (configPage2.nCylinders<=(uint8_t)IGN_CHANNELS);
    if (oddFireSequential || evenFireSequential)
    {
      CRANK_ANGLE_MAX_IGN = 720;
    }
  }
}

void initialiseIgnitionSchedulers(void)
{
  resetIgnitionSchedulers();
  turnOffCoils();
  initialiseIgnitionContext();
  setIgnitionChannelAngles();
  setIgnitionScheduleCallbacks();  
  startIgnitionSchedulers();
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
    if ( maxInjOutputs >= 1U ) { _setFuelScheduleDuration(fuelSchedule1, 100, primingValue); }
#if (INJ_CHANNELS >= 2)
    if ( maxInjOutputs >= 2U ) { _setFuelScheduleDuration(fuelSchedule2, 100, primingValue); }
#endif
#if (INJ_CHANNELS >= 3)
    if ( maxInjOutputs >= 3U ) { _setFuelScheduleDuration(fuelSchedule3, 100, primingValue); }
#endif
#if (INJ_CHANNELS >= 4)
    if ( maxInjOutputs >= 4U ) { _setFuelScheduleDuration(fuelSchedule4, 100, primingValue); }
#endif
#if (INJ_CHANNELS >= 5)
    if ( maxInjOutputs >= 5U ) { _setFuelScheduleDuration(fuelSchedule5, 100, primingValue); }
#endif
#if (INJ_CHANNELS >= 6)
    if ( maxInjOutputs >= 6U ) { _setFuelScheduleDuration(fuelSchedule6, 100, primingValue); }
#endif
#if (INJ_CHANNELS >= 7)
    if ( maxInjOutputs >= 7U ) { _setFuelScheduleDuration(fuelSchedule7, 100, primingValue); }
#endif
#if (INJ_CHANNELS >= 8)
    if ( maxInjOutputs >= 8U ) { _setFuelScheduleDuration(fuelSchedule8, 100, primingValue); }
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

static inline bool isAnyFuelScheduleRunning(void) {
  return isRunning(fuelSchedule1)
      || isRunning(fuelSchedule2)
      || isRunning(fuelSchedule3)
      || isRunning(fuelSchedule4)
#if INJ_CHANNELS >= 5      
      || isRunning(fuelSchedule5)
#endif
#if INJ_CHANNELS >= 6
      || isRunning(fuelSchedule6)
#endif
#if INJ_CHANNELS >= 7
      || isRunning(fuelSchedule7)
#endif
#if INJ_CHANNELS >= 8
      || isRunning(fuelSchedule8)
#endif
      ;
}

static inline bool isAnyIgnScheduleRunning(void) {
  return isRunning(ignitionSchedule1)      
#if IGN_CHANNELS >= 2 
      || isRunning(ignitionSchedule2)
#endif      
#if IGN_CHANNELS >= 3 
      || isRunning(ignitionSchedule3)
#endif      
#if IGN_CHANNELS >= 4       
      || isRunning(ignitionSchedule4)
#endif      
#if IGN_CHANNELS >= 5      
      || isRunning(ignitionSchedule5)
#endif
#if IGN_CHANNELS >= 6
      || isRunning(ignitionSchedule6)
#endif
#if IGN_CHANNELS >= 7
      || isRunning(ignitionSchedule7)
#endif
#if IGN_CHANNELS >= 8
      || isRunning(ignitionSchedule8)
#endif
      ;
}

/** Change injectors or/and ignition angles to 720deg.
 * Roll back req_fuel size and set number of outputs equal to cylinder count.
* */
void changeHalfToFullSync(void)
{
  //Need to do another check for injLayout as this function can be called from ignition
  ATOMIC() {
    if( (configPage2.injLayout == INJ_SEQUENTIAL) && (CRANK_ANGLE_MAX_INJ != 720) && (!isAnyFuelScheduleRunning()))
    {
      CRANK_ANGLE_MAX_INJ = 720;
      req_fuel_uS *= 2;
      
      setCallbacks(fuelSchedule1, openInjector1, closeInjector1);
      setCallbacks(fuelSchedule2, openInjector2, closeInjector2);
      setCallbacks(fuelSchedule3, openInjector3, closeInjector3);
      setCallbacks(fuelSchedule4, openInjector4, closeInjector4);
  #if INJ_CHANNELS >= 5
      setCallbacks(fuelSchedule5, openInjector5, closeInjector5);
  #endif
  #if INJ_CHANNELS >= 6
      setCallbacks(fuelSchedule6, openInjector6, closeInjector6);
  #endif
  #if INJ_CHANNELS >= 7
      setCallbacks(fuelSchedule7, openInjector7, closeInjector7);
  #endif
  #if INJ_CHANNELS >= 8
      setCallbacks(fuelSchedule8, openInjector8, closeInjector8);
  #endif

      switch (configPage2.nCylinders)
      {
        case 4:
          maxInjOutputs = 4;
          break;
              
        case 6:
          maxInjOutputs = 6;
          break;

        case 8:
          maxInjOutputs = 8;
          break;

        default:
          break; //No actions required for other cylinder counts

      }
    }
  }

  //Need to do another check for sparkMode as this function can be called from injection
  ATOMIC() {
    if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (CRANK_ANGLE_MAX_IGN != 720) && (!isAnyIgnScheduleRunning()) )
    {
      CRANK_ANGLE_MAX_IGN = 720;
      maxIgnOutputs = configPage2.nCylinders;
      switch (configPage2.nCylinders)
      {
      case 4:
        setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
        break;

      case 6:
        setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
        setCallbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge);
        break;

      case 8:
        setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
        setCallbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge);
        setCallbacks(ignitionSchedule4, beginCoil4Charge, endCoil4Charge);
        break;

      default:
        break; //No actions required for other cylinder counts
        
      }
    }
  }
}

/** Change injectors or/and ignition angles to 360deg.
 * In semi sequentiol mode req_fuel size is half.
 * Set number of outputs equal to half cylinder count.
* */
void changeFullToHalfSync(void)
{
  if(configPage2.injLayout == INJ_SEQUENTIAL)
  {
    ATOMIC() {
      CRANK_ANGLE_MAX_INJ = 360;
      req_fuel_uS /= 2;
      switch (configPage2.nCylinders)
      {
        case 4:
          if(configPage4.inj4cylPairing == INJ_PAIR_13_24)
          {
            setCallbacks(fuelSchedule1, openInjector1and3, closeInjector1and3);
            setCallbacks(fuelSchedule2, openInjector2and4, closeInjector2and4);
          }
          else
          {
            setCallbacks(fuelSchedule1, openInjector1and4, closeInjector1and4);
            setCallbacks(fuelSchedule2, openInjector2and3, closeInjector2and3);
          }
          maxInjOutputs = 2;
          break;
              
        case 6:
          setCallbacks(fuelSchedule1, openInjector1and4, closeInjector1and4);
          setCallbacks(fuelSchedule2, openInjector2and5, closeInjector2and5);
          setCallbacks(fuelSchedule3, openInjector3and6, closeInjector3and6);
          maxInjOutputs = 3;
          break;

        case 8:
          setCallbacks(fuelSchedule1, openInjector1and5, closeInjector1and5);
          setCallbacks(fuelSchedule2, openInjector2and6, closeInjector2and6);
          setCallbacks(fuelSchedule3, openInjector3and7, closeInjector3and7);
          setCallbacks(fuelSchedule4, openInjector4and8, closeInjector4and8);
          maxInjOutputs = 4;
          break;

        default: break;
      }
    }
  }

  if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
  {
    ATOMIC() {
      CRANK_ANGLE_MAX_IGN = 360;
      maxIgnOutputs = configPage2.nCylinders / 2;
      switch (configPage2.nCylinders)
      {
        case 4:
          setCallbacks(ignitionSchedule1, beginCoil1and3Charge, endCoil1and3Charge);
          setCallbacks(ignitionSchedule2, beginCoil2and4Charge, endCoil2and4Charge);
          break;
              
        case 6:
          setCallbacks(ignitionSchedule1, beginCoil1and4Charge, endCoil1and4Charge);
          setCallbacks(ignitionSchedule2, beginCoil2and5Charge, endCoil2and5Charge);
          setCallbacks(ignitionSchedule3, beginCoil3and6Charge, endCoil3and6Charge);
          break;

        case 8:
          setCallbacks(ignitionSchedule1, beginCoil1and5Charge, endCoil1and5Charge);
          setCallbacks(ignitionSchedule2, beginCoil2and6Charge, endCoil2and6Charge);
          setCallbacks(ignitionSchedule3, beginCoil3and7Charge, endCoil3and7Charge);
          setCallbacks(ignitionSchedule4, beginCoil4and8Charge, endCoil4and8Charge);
          break;
        default: break;
      }
    }
  }
}
