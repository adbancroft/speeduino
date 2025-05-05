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
#include "utilities.h"
#include "static_for.hpp"

// cppcheck-suppress misra-c2012-9.3
FuelSchedule fuelSchedules[INJ_CHANNELS] = {
  FuelSchedule(FUEL1_COUNTER, FUEL1_COMPARE),
#if (INJ_CHANNELS >= 2)
  FuelSchedule(FUEL2_COUNTER, FUEL2_COMPARE),
#endif
#if (INJ_CHANNELS >= 3)
  FuelSchedule(FUEL3_COUNTER, FUEL3_COMPARE),
#endif
#if (INJ_CHANNELS >= 4)
  FuelSchedule(FUEL4_COUNTER, FUEL4_COMPARE),
#endif
#if (INJ_CHANNELS >= 5)
  FuelSchedule(FUEL5_COUNTER, FUEL5_COMPARE),
#endif
#if (INJ_CHANNELS >= 6)
  FuelSchedule(FUEL6_COUNTER, FUEL6_COMPARE),
#endif
#if (INJ_CHANNELS >= 7)
  FuelSchedule(FUEL7_COUNTER, FUEL7_COMPARE),
#endif
#if (INJ_CHANNELS >= 8)
  FuelSchedule(FUEL8_COUNTER, FUEL8_COMPARE),
#endif
};

// cppcheck-suppress misra-c2012-9.3
IgnitionSchedule ignitionSchedules[IGN_CHANNELS]  = {
  IgnitionSchedule(IGN1_COUNTER, IGN1_COMPARE),
#if IGN_CHANNELS >= 2
  IgnitionSchedule(IGN2_COUNTER, IGN2_COMPARE),
#endif
#if IGN_CHANNELS >= 3
  IgnitionSchedule(IGN3_COUNTER, IGN3_COMPARE),
#endif
#if IGN_CHANNELS >= 4
  IgnitionSchedule(IGN4_COUNTER, IGN4_COMPARE),
#endif
#if IGN_CHANNELS >= 5
  IgnitionSchedule(IGN5_COUNTER, IGN5_COMPARE),
#endif
#if IGN_CHANNELS >= 6
  IgnitionSchedule(IGN6_COUNTER, IGN6_COMPARE),
#endif
#if IGN_CHANNELS >= 7
  IgnitionSchedule(IGN7_COUNTER, IGN7_COMPARE),
#endif
#if IGN_CHANNELS >= 8
  IgnitionSchedule(IGN8_COUNTER, IGN8_COMPARE),
#endif
};

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
  for (uint8_t index=0U; index<_countof(fuelSchedules); ++index) {
    reset(fuelSchedules[index]);
  }
}

void resetIgnitionSchedulers(void)
{
  for (uint8_t index=0U; index<_countof(ignitionSchedules); ++index) {
    reset(ignitionSchedules[index]);
  }
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

static void initialiseStagedInjection(void) {
  maxInjSecondaryOutputs = 0;

  // Check if injector staging is enabled && we have spare injector channels
  if((configPage10.stagingEnabled == true) && (INJ_CHANNELS>maxInjPrimaryOutputs))
  {
    // If we have enough channels, use the same # of secondaries as primaries.
    // E.g. 4 cylinder sequential with 8 channels -> 4 secondary
    //      4 cylinder paired with 4 channels -> 2 secondary
    //      3 cylinder paired with 8 channels -> 3 secondary (and 2 unused)
    if ((INJ_CHANNELS-maxInjPrimaryOutputs)>=maxInjPrimaryOutputs) {
      maxInjSecondaryOutputs = maxInjPrimaryOutputs;
    // Not quite enough channels, use a single secondary.
    // E.g. 3 cylinder sequential with 4 channels -> 1 secondary
    //      4 cylinder sequential with 5 channels -> 1 secondary
    } else if (configPage2.nCylinders!=6) {
      maxInjSecondaryOutputs = 1;
    }

    for (uint8_t index = 0; index<maxInjSecondaryOutputs; ++index) {
      fuelSchedules[index+maxInjPrimaryOutputs].channelDegrees = fuelSchedules[index].channelDegrees;
    }

    // Special case for 2 cylinder
    if(configPage2.nCylinders==2U)
    {
      //Phase this either 180 or 360 degrees out from inj3 (In reality this will always be 180 as you can't have sequential and staged currently)
      fuelSchedules[3].channelDegrees = fuelSchedules[2].channelDegrees + (uint16_t)(CRANK_ANGLE_MAX_INJ / 2U); 
      if (fuelSchedules[3].channelDegrees>=(uint16_t)CRANK_ANGLE_MAX_INJ) { fuelSchedules[3].channelDegrees -= (uint16_t)CRANK_ANGLE_MAX_INJ; }
    }    
  }
}

static inline bool isSequentialInjectionOn(void) {
  // Sequential if asked for & there is enough channels
  return (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) && (INJ_CHANNELS>=configPage2.nCylinders);
}

static void setFuelChannelAngles(void)
{
  // Sequential if asked for & there is enough channels
  bool isSequential = isSequentialInjectionOn();
  // Adjust angles by # of squirts?
  // bool useSquirts =     (!isSequential) && (configPage2.injTiming) && (currentStatus.nSquirts > 2)
  //                   // No idea why, but the original code didn't account for squirts with 5 cylinders
  //                   &&  (configPage2.nCylinders!=5); 

  // Calculate # of primary injectors
  if (configPage2.nCylinders==4 || configPage2.nCylinders==6 || configPage2.nCylinders==8) {
    maxInjPrimaryOutputs = isSequential ? configPage2.nCylinders : configPage2.nCylinders/2;
  } else {
    // 1, 2, 3 & 5 cylinder are essentially sequential if we have enough channels
    maxInjPrimaryOutputs = min((uint8_t)configPage2.nCylinders, (uint8_t)INJ_CHANNELS);
  }

  // Caclulate degrees between squirts
  uint16_t spacing = 0U;
  if (configPage2.injTiming) {
    // Oddfire only supported on 2 cylinders
    if (configPage2.nCylinders==2U) {
      spacing = configPage2.engineType == EVEN_FIRE ?  CRANK_ANGLE_MAX_INJ / 2U : configPage2.oddfire2;
    } else {
      if ((configPage2.injLayout == INJ_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) && (configPage2.nCylinders>maxInjPrimaryOutputs)) {
        spacing = CRANK_ANGLE_MAX_INJ / configPage2.nCylinders;
      } else {
        spacing = CRANK_ANGLE_MAX_INJ / maxInjPrimaryOutputs;
      }
    }
  }

  // Compute channel angles
  for (uint8_t index=0U; index<maxInjPrimaryOutputs; ++index) {
    fuelSchedules[index].channelDegrees = index * spacing;
    // if (useSquirts) {
    //   fuelSchedules[index].channelDegrees = (fuelSchedules[index].channelDegrees * 2U) / currentStatus.nSquirts;
    // }
  }
}

static void setFuelScheduleCallbacks(void)
{
  switch(configPage2.injLayout)
  {
  case INJ_PAIRED:
      //Paired injection
      setCallbacks(fuelSchedules[0], openInjector1, closeInjector1);
      setCallbacks(fuelSchedules[1], openInjector2, closeInjector2);
      setCallbacks(fuelSchedules[2], openInjector3, closeInjector3);
      setCallbacks(fuelSchedules[3], openInjector4, closeInjector4);
  #if INJ_CHANNELS >= 5
      setCallbacks(fuelSchedules[4], openInjector5, closeInjector5);
  #endif
      break;

  case INJ_SEMISEQUENTIAL:
      //Semi-Sequential injection. Currently possible with 4, 6 and 8 cylinders. 5 cylinder is a special case
      if( configPage2.nCylinders == 4U )
      {
        if(configPage4.inj4cylPairing == INJ_PAIR_13_24)
        {
          setCallbacks(fuelSchedules[0], openInjector1and3, closeInjector1and3);
          setCallbacks(fuelSchedules[1], openInjector2and4, closeInjector2and4);
        }
        else
        {
          setCallbacks(fuelSchedules[0], openInjector1and4, closeInjector1and4);
          setCallbacks(fuelSchedules[1], openInjector2and3, closeInjector2and3);
        }
      }
      else if( configPage2.nCylinders == 5U ) //This is similar to the paired injection but uses five injector outputs instead of four
      {
        setCallbacks(fuelSchedules[0], openInjector1, closeInjector1);
        setCallbacks(fuelSchedules[1], openInjector2, closeInjector2);
        setCallbacks(fuelSchedules[2], openInjector3and5, closeInjector3and5);
        setCallbacks(fuelSchedules[3], openInjector4, closeInjector4);
      }
      else if( configPage2.nCylinders == 6U )
      {
        setCallbacks(fuelSchedules[0], openInjector1and4, closeInjector1and4);
        setCallbacks(fuelSchedules[1], openInjector2and5, closeInjector2and5);
        setCallbacks(fuelSchedules[2], openInjector3and6, closeInjector3and6);
      }
      else if( configPage2.nCylinders == 8U )
      {
        setCallbacks(fuelSchedules[0], openInjector1and5, closeInjector1and5);
        setCallbacks(fuelSchedules[1], openInjector2and6, closeInjector2and6);
        setCallbacks(fuelSchedules[2], openInjector3and7, closeInjector3and7);
        setCallbacks(fuelSchedules[3], openInjector4and8, closeInjector4and8);
      }
      else
      {
        //Fall back to paired injection
        setCallbacks(fuelSchedules[0], openInjector1, closeInjector1);
        setCallbacks(fuelSchedules[1], openInjector2, closeInjector2);
        setCallbacks(fuelSchedules[2], openInjector3, closeInjector3);
        setCallbacks(fuelSchedules[3], openInjector4, closeInjector4);
  #if INJ_CHANNELS >= 5
        setCallbacks(fuelSchedules[4], openInjector5, closeInjector5);
  #endif
      }
      break;

  case INJ_SEQUENTIAL:
      //Sequential injection
      setCallbacks(fuelSchedules[0], openInjector1, closeInjector1);
      setCallbacks(fuelSchedules[1], openInjector2, closeInjector2);
      setCallbacks(fuelSchedules[2], openInjector3, closeInjector3);
      setCallbacks(fuelSchedules[3], openInjector4, closeInjector4);
  #if INJ_CHANNELS >= 5
      setCallbacks(fuelSchedules[4], openInjector5, closeInjector5);
  #endif
  #if INJ_CHANNELS >= 6
      setCallbacks(fuelSchedules[5], openInjector6, closeInjector6);
  #endif
  #if INJ_CHANNELS >= 7
      setCallbacks(fuelSchedules[6], openInjector7, closeInjector7);
  #endif
  #if INJ_CHANNELS >= 8
      setCallbacks(fuelSchedules[7], openInjector8, closeInjector8);
  #endif
      break;

  default:
      //Paired injection
      setCallbacks(fuelSchedules[0], openInjector1, closeInjector1);
      setCallbacks(fuelSchedules[1], openInjector2, closeInjector2);
      setCallbacks(fuelSchedules[2], openInjector3, closeInjector3);
      setCallbacks(fuelSchedules[3], openInjector4, closeInjector4);
  #if INJ_CHANNELS >= 5
      setCallbacks(fuelSchedules[4], openInjector5, closeInjector5);
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

  if(  (configPage2.nCylinders==3) 
    && (configPage2.injType == INJ_TYPE_PORT)
    && (configPage2.injLayout != INJ_SEQUENTIAL) ) {
      //Force nSquirts to 2 for individual port injection. This prevents TunerStudio forcing the value to 3 even when this isn't wanted. 
      currentStatus.nSquirts = 2;
      CRANK_ANGLE_MAX_INJ = (configPage2.strokes == FOUR_STROKE) ? 360 : 180;
  }
  else if ( (configPage2.injLayout == INJ_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) && (configPage2.nCylinders<=(uint8_t)INJ_CHANNELS))
  {
    CRANK_ANGLE_MAX_INJ = 720U;
    currentStatus.nSquirts = 1U;
    req_fuel_uS = req_fuel_uS * 2U;
  }
  else
  {
    if ((configPage2.injLayout == INJ_SEQUENTIAL) && configPage2.nCylinders==3U) {
      currentStatus.nSquirts = 1;
    } else {
      currentStatus.nSquirts = max(1U, configPage2.nCylinders / configPage2.divider);
    }

    CRANK_ANGLE_MAX_INJ = (configPage2.strokes == FOUR_STROKE ? 720U : 360U) / currentStatus.nSquirts;
  }

  //Special case:
  //3 or 5 squirts per cycle MUST be tracked over 720 degrees. This is because the angles for them (Eg 720/3=240) are not evenly divisible into 360
  //This is ONLY the case on 4 stroke systems
  if( ((currentStatus.nSquirts == 3U) || (currentStatus.nSquirts == 5U)) && (configPage2.strokes == FOUR_STROKE))
  {
    CRANK_ANGLE_MAX_INJ = 720U/currentStatus.nSquirts;
  } 

  currentStatus.status3 |= currentStatus.nSquirts << BIT_STATUS3_NSQUIRTS1; //Top 3 bits of the status3 variable are the number of squirts. This must be done after the above section due to nSquirts being forced to 1 for sequential
}

void initialiseFuelSchedulers(void)
{
  resetFuelSchedulers();
  turnOffInjectors();
  initialiseFuelContext();
  setFuelChannelAngles();
  initialiseStagedInjection();
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
      ignitionSchedules[0].channelDegrees = 0;
      maxIgnOutputs = 1;
      break;

  case 2:
      ignitionSchedules[0].channelDegrees = 0;
      maxIgnOutputs = 2;
      if (configPage2.engineType == EVEN_FIRE ) { ignitionSchedules[1].channelDegrees = 180; }
      else { ignitionSchedules[1].channelDegrees = configPage2.oddfire2; }
      break;

  case 3:
      ignitionSchedules[0].channelDegrees = 0;
      maxIgnOutputs = 3;
      if (configPage2.engineType == EVEN_FIRE )
      {
        //Sequential and Single channel modes both run over 720 crank degrees, but only on 4 stroke engines.
        if( ( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) || (configPage4.sparkMode == IGN_MODE_SINGLE) ) && (configPage2.strokes == FOUR_STROKE) )
        {
          ignitionSchedules[1].channelDegrees = 240;
          ignitionSchedules[2].channelDegrees = 480;
        }
        else
        {
          ignitionSchedules[1].channelDegrees = 120;
          ignitionSchedules[2].channelDegrees = 240;
        }
      }
      else
      {
        ignitionSchedules[1].channelDegrees = configPage2.oddfire2;
        ignitionSchedules[2].channelDegrees = configPage2.oddfire3;
      }
      break;

  case 4:
      ignitionSchedules[0].channelDegrees = 0;
      maxIgnOutputs = 2; //Default value for 4 cylinder, may be changed below
      if (configPage2.engineType == EVEN_FIRE )
      {
        ignitionSchedules[1].channelDegrees = 180;

        if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) && (configPage2.strokes == FOUR_STROKE) )
        {
          ignitionSchedules[2].channelDegrees = 360;
          ignitionSchedules[3].channelDegrees = 540;
          maxIgnOutputs = 4;
        }
        if(configPage4.sparkMode == IGN_MODE_ROTARY)
        {
          //Rotary uses the ign 3 and 4 schedules for the trailing spark. They are offset from the ign 1 and 2 channels respectively and so use the same degrees as them
          ignitionSchedules[2].channelDegrees = 0;
          ignitionSchedules[3].channelDegrees = 180;
          maxIgnOutputs = 4;

          configPage4.IgInv = GOING_LOW; //Force Going Low ignition mode (Going high is never used for rotary)
        }
      }
      else
      {
        ignitionSchedules[1].channelDegrees = configPage2.oddfire2;
        ignitionSchedules[2].channelDegrees = configPage2.oddfire3;
        ignitionSchedules[3].channelDegrees = configPage2.oddfire4;
        maxIgnOutputs = 4;
      }
      break;

  case 5:
      ignitionSchedules[0].channelDegrees = 0;
      ignitionSchedules[1].channelDegrees = 72;
      ignitionSchedules[2].channelDegrees = 144;
      ignitionSchedules[3].channelDegrees = 216;
#if IGN_CHANNELS >= 5
      ignitionSchedules[4].channelDegrees = 288;
      maxIgnOutputs = 5; //Only 4 actual outputs, so that's all that can be cut
#else
      maxIgnOutputs = 4; //Only 4 actual outputs, so that's all that can be cut
#endif

      if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
      {
        ignitionSchedules[1].channelDegrees = 144;
        ignitionSchedules[2].channelDegrees = 288;
        ignitionSchedules[3].channelDegrees = 432;
#if IGN_CHANNELS >= 5
        ignitionSchedules[4].channelDegrees = 576;
        maxIgnOutputs = 5; //Only 4 actual outputs, so that's all that can be cut
#endif
      }
      break;

  case 6:
      ignitionSchedules[0].channelDegrees = 0;
      ignitionSchedules[1].channelDegrees = 120;
      ignitionSchedules[2].channelDegrees = 240;
      maxIgnOutputs = 3;

  #if IGN_CHANNELS >= 6
      if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL))
      {
        ignitionSchedules[3].channelDegrees = 360;
        ignitionSchedules[4].channelDegrees = 480;
        ignitionSchedules[5].channelDegrees = 600;
        maxIgnOutputs = 6;
      }
  #endif
      break;

  case 8:
      ignitionSchedules[0].channelDegrees = 0;
      ignitionSchedules[1].channelDegrees = 90;
      ignitionSchedules[2].channelDegrees = 180;
      ignitionSchedules[3].channelDegrees = 270;
      maxIgnOutputs = 4;

      if(configPage4.sparkMode == IGN_MODE_SINGLE)
      {
        maxIgnOutputs = 4;
      }
  
  #if IGN_CHANNELS >= 8
      if( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL))
      {
        ignitionSchedules[4].channelDegrees = 360;
        ignitionSchedules[5].channelDegrees = 450;
        ignitionSchedules[6].channelDegrees = 540;
        ignitionSchedules[7].channelDegrees = 630;
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
      setCallbacks(ignitionSchedules[0], beginCoil1Charge, endCoil1Charge);
      setCallbacks(ignitionSchedules[1], beginCoil2Charge, endCoil2Charge);
      setCallbacks(ignitionSchedules[2], beginCoil3Charge, endCoil3Charge);
      setCallbacks(ignitionSchedules[3], beginCoil4Charge, endCoil4Charge);
#if IGN_CHANNELS >= 5
        setCallbacks(ignitionSchedules[4], beginCoil5Charge, endCoil5Charge);
#endif
      break;

  case IGN_MODE_SINGLE:
      //Single channel mode. All ignition pulses are on channel 1
      setCallbacks(ignitionSchedules[0], beginCoil1Charge, endCoil1Charge);
      setCallbacks(ignitionSchedules[1], beginCoil1Charge, endCoil1Charge);
      setCallbacks(ignitionSchedules[2], beginCoil1Charge, endCoil1Charge);
      setCallbacks(ignitionSchedules[3], beginCoil1Charge, endCoil1Charge);
#if IGN_CHANNELS >= 5
        setCallbacks(ignitionSchedules[4], beginCoil1Charge, endCoil1Charge);
#endif
#if IGN_CHANNELS >= 6
      setCallbacks(ignitionSchedules[5], beginCoil1Charge, endCoil1Charge);
#endif
#if IGN_CHANNELS >= 7
      setCallbacks(ignitionSchedules[6], beginCoil1Charge, endCoil1Charge);
#endif
#if IGN_CHANNELS >= 8
      setCallbacks(ignitionSchedules[7], beginCoil1Charge, endCoil1Charge);
#endif
      break;

  case IGN_MODE_WASTEDCOP:
      //Wasted COP mode. Note, most of the boards can only run this for 4-cyl only.
      if( configPage2.nCylinders <= 3U)
      {
          //1-3 cylinder wasted COP is the same as regular wasted mode
        setCallbacks(ignitionSchedules[0], beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedules[1], beginCoil2Charge, endCoil2Charge);
        setCallbacks(ignitionSchedules[2], beginCoil3Charge, endCoil3Charge);
        setCallbacks(ignitionSchedules[3], beginCoil4Charge, endCoil4Charge);          
      }
      else if( configPage2.nCylinders == 4U )
      {
        //Wasted COP mode for 4 cylinders. Ignition channels 1&3 and 2&4 are paired together
        setCallbacks(ignitionSchedules[0], beginCoil1and3Charge, endCoil1and3Charge);
        setCallbacks(ignitionSchedules[1], beginCoil2and4Charge, endCoil2and4Charge);

        setCallbacks(ignitionSchedules[2], nullCallback, nullCallback);
        setCallbacks(ignitionSchedules[3], nullCallback, nullCallback);
      }
      else if( configPage2.nCylinders == 6U )
      {
        //Wasted COP mode for 6 cylinders. Ignition channels 1&4, 2&5 and 3&6 are paired together
        setCallbacks(ignitionSchedules[0], beginCoil1and4Charge, endCoil1and4Charge);
        setCallbacks(ignitionSchedules[1], beginCoil2and5Charge, endCoil2and5Charge);
        setCallbacks(ignitionSchedules[2], beginCoil3and6Charge, endCoil3and6Charge);
        setCallbacks(ignitionSchedules[3], nullCallback, nullCallback);
#if IGN_CHANNELS >= 5
        setCallbacks(ignitionSchedules[4], nullCallback, nullCallback);
#endif
#if IGN_CHANNELS >= 6
        setCallbacks(ignitionSchedules[5], nullCallback, nullCallback);
#endif
      }
      else if( configPage2.nCylinders == 8U )
      {
        //Wasted COP mode for 8 cylinders. Ignition channels 1&5, 2&6, 3&7 and 4&8 are paired together
        setCallbacks(ignitionSchedules[0], beginCoil1and5Charge, endCoil1and5Charge);
        setCallbacks(ignitionSchedules[1], beginCoil2and6Charge, endCoil2and6Charge);
        setCallbacks(ignitionSchedules[2], beginCoil3and7Charge, endCoil3and7Charge);
        setCallbacks(ignitionSchedules[3], beginCoil4and8Charge, endCoil4and8Charge);
#if IGN_CHANNELS >= 5
        setCallbacks(ignitionSchedules[4], nullCallback, nullCallback);
#endif
#if IGN_CHANNELS >= 6
        setCallbacks(ignitionSchedules[5], nullCallback, nullCallback);
#endif
#if IGN_CHANNELS >= 7
        setCallbacks(ignitionSchedules[6], nullCallback, nullCallback);
#endif
#if IGN_CHANNELS >= 8
        setCallbacks(ignitionSchedules[7], nullCallback, nullCallback);
#endif
      }
      else
      {
        //If the person has inadvertently selected this when running more than 4 cylinders or other than 6 cylinders, just use standard Wasted spark mode
        setCallbacks(ignitionSchedules[0], beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedules[1], beginCoil2Charge, endCoil2Charge);
        setCallbacks(ignitionSchedules[2], beginCoil3Charge, endCoil3Charge);
        setCallbacks(ignitionSchedules[3], beginCoil4Charge, endCoil4Charge);
#if IGN_CHANNELS >= 5
        setCallbacks(ignitionSchedules[4], beginCoil5Charge, endCoil5Charge);
#endif
      }
      break;

  case IGN_MODE_SEQUENTIAL:
      setCallbacks(ignitionSchedules[0], beginCoil1Charge, endCoil1Charge);
      setCallbacks(ignitionSchedules[1], beginCoil2Charge, endCoil2Charge);
      setCallbacks(ignitionSchedules[2], beginCoil3Charge, endCoil3Charge);
      setCallbacks(ignitionSchedules[3], beginCoil4Charge, endCoil4Charge);
#if IGN_CHANNELS >= 5
      setCallbacks(ignitionSchedules[4], beginCoil5Charge, endCoil5Charge);
#endif
#if IGN_CHANNELS >= 6
      setCallbacks(ignitionSchedules[5], beginCoil6Charge, endCoil6Charge);
#endif
#if IGN_CHANNELS >= 7
      setCallbacks(ignitionSchedules[6], beginCoil7Charge, endCoil7Charge);
#endif
#if IGN_CHANNELS >= 8
      setCallbacks(ignitionSchedules[7], beginCoil8Charge, endCoil8Charge);
#endif
      break;

  case IGN_MODE_ROTARY:
      if(configPage10.rotaryType == ROTARY_IGN_FC)
      {
        //Ignition channel 1 is a wasted spark signal for leading signal on both rotors
        setCallbacks(ignitionSchedules[0], beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedules[1], beginCoil1Charge, endCoil1Charge);

        setCallbacks(ignitionSchedules[2], beginTrailingCoilCharge, endTrailingCoilCharge1);
        setCallbacks(ignitionSchedules[3], beginTrailingCoilCharge, endTrailingCoilCharge2);
      }
      else if(configPage10.rotaryType == ROTARY_IGN_FD)
      {
        //Ignition channel 1 is a wasted spark signal for leading signal on both rotors
        setCallbacks(ignitionSchedules[0], beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedules[1], beginCoil1Charge, endCoil1Charge);

        //Trailing coils have their own channel each
        //IGN2 = front rotor trailing spark
        setCallbacks(ignitionSchedules[2], beginCoil2Charge, endCoil2Charge);
        //IGN3 = rear rotor trailing spark
        setCallbacks(ignitionSchedules[3], beginCoil3Charge, endCoil3Charge);

        //IGN4 not used
      }
      else if(configPage10.rotaryType == ROTARY_IGN_RX8)
      {
        //RX8 outputs are simply 1 coil and 1 output per plug

        //IGN1 is front rotor, leading spark
        setCallbacks(ignitionSchedules[0], beginCoil1Charge, endCoil1Charge);
        //IGN2 is rear rotor, leading spark
        setCallbacks(ignitionSchedules[1], beginCoil2Charge, endCoil2Charge);
        //IGN3 = front rotor trailing spark
        setCallbacks(ignitionSchedules[2], beginCoil3Charge, endCoil3Charge);
        //IGN4 = rear rotor trailing spark
        setCallbacks(ignitionSchedules[3], beginCoil4Charge, endCoil4Charge);
      }
      else { } //No action for other RX ignition modes (Future expansion / MISRA compliant). 
      break;

  default:
      //Wasted spark (Shouldn't ever happen anyway)
      setCallbacks(ignitionSchedules[0], beginCoil1Charge, endCoil1Charge);
      setCallbacks(ignitionSchedules[1], beginCoil2Charge, endCoil2Charge);
      setCallbacks(ignitionSchedules[2], beginCoil3Charge, endCoil3Charge);
      setCallbacks(ignitionSchedules[3], beginCoil4Charge, endCoil4Charge);
#if IGN_CHANNELS >= 5
      setCallbacks(ignitionSchedules[4], beginCoil5Charge, endCoil5Charge);
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
    for (uint8_t index=0U; index<totalInjOutputs(); ++index) {
      _setFuelScheduleDuration(fuelSchedules[index], 100U, primingValue); 
    }
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


TESTABLE_INLINE_STATIC void applyChannelOverDwellProtection(IgnitionSchedule &schedule, uint32_t targetOverdwellTime) {
  //Check first whether each spark output is currently on. Only check it's dwell time if it is
  ATOMIC() {
    if (isRunning(schedule) && unlikely(schedule._startTime < targetOverdwellTime)) { 
      ignitionRunningToOff(&schedule); //Call the end function to disable the spark output
    }
  }
}

static inline void applyChannelOverDwellProtectionIndex(uint8_t index, uint32_t targetOverdwellTime) {
  applyChannelOverDwellProtection(ignitionSchedules[index], targetOverdwellTime);
}

TESTABLE_INLINE_STATIC bool isOverDwellActive(const config4 &page4, const statuses &current){
  bool isCrankLocked = page4.ignCranklock && (current.RPM < current.crankRPM); //Dwell limiter is disabled during cranking on setups using the locked cranking timing. WE HAVE to do the RPM check here as relying on the engine cranking bit can be potentially too slow in updating
  return (page4.useDwellLim) && likely(!isCrankLocked);
}

void applyOverDwellProtection(void)
{
  if (likely(isOverDwellActive(configPage4, currentStatus))) {
    uint32_t targetOverdwellTime = micros() - (configPage4.dwellLimit * 1000U); //Convert to uS

    ATOMIC() {
      static_for<0U, _countof(ignitionSchedules)>::repeat_n(applyChannelOverDwellProtectionIndex, targetOverdwellTime);
    }
  }
}

static inline bool isAnyFuelScheduleRunning(void) {
  uint8_t index=0U;
  while (index<_countof(fuelSchedules) && !isRunning(fuelSchedules[index])) {
    ++index;
  }
   
  return index<_countof(fuelSchedules);
}

static inline bool isAnyIgnScheduleRunning(void) {
  uint8_t index=0U;
  while (index<_countof(ignitionSchedules) && !isRunning(ignitionSchedules[index])) {
    ++index;
  }
   
  return index<_countof(ignitionSchedules);
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
    maxInjPrimaryOutputs = configPage2.nCylinders;
      req_fuel_uS *= 2;
      
      setCallbacks(fuelSchedules[0], openInjector1, closeInjector1);
      setCallbacks(fuelSchedules[1], openInjector2, closeInjector2);
      setCallbacks(fuelSchedules[2], openInjector3, closeInjector3);
      setCallbacks(fuelSchedules[3], openInjector4, closeInjector4);
  #if INJ_CHANNELS >= 5
      setCallbacks(fuelSchedules[4], openInjector5, closeInjector5);
  #endif
  #if INJ_CHANNELS >= 6
      setCallbacks(fuelSchedules[5], openInjector6, closeInjector6);
  #endif
  #if INJ_CHANNELS >= 7
      setCallbacks(fuelSchedules[6], openInjector7, closeInjector7);
  #endif
  #if INJ_CHANNELS >= 8
      setCallbacks(fuelSchedules[7], openInjector8, closeInjector8);
  #endif
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
        setCallbacks(ignitionSchedules[0], beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedules[1], beginCoil2Charge, endCoil2Charge);
        break;

      case 6:
        setCallbacks(ignitionSchedules[0], beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedules[1], beginCoil2Charge, endCoil2Charge);
        setCallbacks(ignitionSchedules[2], beginCoil3Charge, endCoil3Charge);
        break;

      case 8:
        setCallbacks(ignitionSchedules[0], beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedules[1], beginCoil2Charge, endCoil2Charge);
        setCallbacks(ignitionSchedules[2], beginCoil3Charge, endCoil3Charge);
        setCallbacks(ignitionSchedules[3], beginCoil4Charge, endCoil4Charge);
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
      maxInjPrimaryOutputs = configPage2.nCylinders / 2;
      switch (configPage2.nCylinders)
      {
        case 4:
          if(configPage4.inj4cylPairing == INJ_PAIR_13_24)
          {
          setCallbacks(fuelSchedules[0], openInjector1and3, closeInjector1and3);
          setCallbacks(fuelSchedules[1], openInjector2and4, closeInjector2and4);
          }
          else
          {
          setCallbacks(fuelSchedules[0], openInjector1and4, closeInjector1and4);
          setCallbacks(fuelSchedules[1], openInjector2and3, closeInjector2and3);
          }
          break;
              
        case 6:
        setCallbacks(fuelSchedules[0], openInjector1and4, closeInjector1and4);
        setCallbacks(fuelSchedules[1], openInjector2and5, closeInjector2and5);
        setCallbacks(fuelSchedules[2], openInjector3and6, closeInjector3and6);
          break;

        case 8:
        setCallbacks(fuelSchedules[0], openInjector1and5, closeInjector1and5);
        setCallbacks(fuelSchedules[1], openInjector2and6, closeInjector2and6);
        setCallbacks(fuelSchedules[2], openInjector3and7, closeInjector3and7);
        setCallbacks(fuelSchedules[3], openInjector4and8, closeInjector4and8);
          break;

        default: break;
      }
    }
  }

  if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
  {
    ATOMIC() {
      CRANK_ANGLE_MAX_IGN = 360;
      maxIgnOutputs = configPage2.nCylinders / 2U;
      switch (configPage2.nCylinders)
      {
        case 4:
        setCallbacks(ignitionSchedules[0], beginCoil1and3Charge, endCoil1and3Charge);
        setCallbacks(ignitionSchedules[1], beginCoil2and4Charge, endCoil2and4Charge);
          break;
              
        case 6:
        setCallbacks(ignitionSchedules[0], beginCoil1and4Charge, endCoil1and4Charge);
        setCallbacks(ignitionSchedules[1], beginCoil2and5Charge, endCoil2and5Charge);
        setCallbacks(ignitionSchedules[2], beginCoil3and6Charge, endCoil3and6Charge);
          break;

        case 8:
        setCallbacks(ignitionSchedules[0], beginCoil1and5Charge, endCoil1and5Charge);
        setCallbacks(ignitionSchedules[1], beginCoil2and6Charge, endCoil2and6Charge);
        setCallbacks(ignitionSchedules[2], beginCoil3and7Charge, endCoil3and7Charge);
        setCallbacks(ignitionSchedules[3], beginCoil4and8Charge, endCoil4and8Charge);
          break;

        default: break;
      }
    }
  }
}
