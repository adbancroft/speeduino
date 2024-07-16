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
#include "scheduler_callbacks.h"
#include "timers.h"
#include "schedule_state_machine.h"
#include "speeduino.h"
#include "utilities.h"
#include "unit_testing.h"

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

InjectorChannels injectorChannels;
byte maxIgnOutputs;

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

TESTABLE_INLINE_STATIC void resetFuelSchedulers(void)
{
  for (uint8_t index=0U; index<_countof(fuelSchedules); ++index) {
    reset(fuelSchedules[index]);
  }
}

TESTABLE_INLINE_STATIC void resetIgnitionSchedulers(void)
{
  for (uint8_t index=0U; index<_countof(ignitionSchedules); ++index) {
    reset(ignitionSchedules[index]);
  }
}

TESTABLE_INLINE_STATIC void startFuelSchedulers(void)
{
  FUEL1_TIMER_ENABLE();
#if INJ_CHANNELS >= 2
  FUEL2_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 3
  FUEL3_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 4
  FUEL4_TIMER_ENABLE();
#endif
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
  for (uint8_t index=0; index<_countof(fuelSchedules); ++index) {
    closeInjector(index+1U);
  }
}

static void initialiseStagedInjection(void) {
  injectorChannels.secondary = 0U;

  // Can't stage unless we have enough channels
  configPage10.stagingEnabled = configPage10.stagingEnabled  && (injectorChannels.primary < _countof(fuelSchedules));

  if (configPage10.stagingEnabled == true)
  {
    // If we have enough channels, use the same # of secondaries as primaries.
    // E.g. 4 cylinder sequential with 8 channels -> 4 secondary
    //      4 cylinder paired with 4 channels -> 2 secondary
    //      3 cylinder paired with 8 channels -> 3 secondary (and 2 unused)
    if ((_countof(fuelSchedules)-injectorChannels.primary)>=injectorChannels.primary) {
      injectorChannels.secondary = injectorChannels.primary;
    // Not quite enough channels, use a single secondary.
    // E.g. 3 cylinder sequential with 4 channels -> 1 secondary
    //      4 cylinder sequential with 5 channels -> 1 secondary
     } else if (configPage2.nCylinders!=6U) {
      injectorChannels.secondary = 1U;
    } else {
      // We don't apply the single secondary rule to 6 cylinder
      // No idea why
    }

    for (uint8_t index = 0U; index<injectorChannels.secondary; ++index) {
      fuelSchedules[index+injectorChannels.primary].channelDegrees = fuelSchedules[index].channelDegrees;
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

static void setFuelChannelAngles(void)
{
  // Calculate # of primary injectors
  if (configPage2.nCylinders==4U || configPage2.nCylinders==6U || configPage2.nCylinders==8U) {
     injectorChannels.primary = configPage2.injLayout == INJ_SEQUENTIAL ? configPage2.nCylinders : configPage2.nCylinders/2U;
  } else {
    // 1, 2, 3 & 5 cylinder are essentially sequential if we have enough channels
    injectorChannels.primary = min((uint8_t)configPage2.nCylinders, (uint8_t)_countof(fuelSchedules));
  }

  // Calculate degrees between squirts
  uint16_t spacing = 0U;
  if (configPage2.injTiming==INJ_TIMING_ALTERNATING) {
    // Oddfire only supported on 2 cylinders
    if ((configPage2.engineType == ODD_FIRE) && (configPage2.nCylinders==2U)) {
      spacing = configPage2.oddfire[0];
    } else {
      spacing = (uint16_t)CRANK_ANGLE_MAX_INJ / injectorChannels.primary;
    }
  }

  // Compute channel angles
  for (uint8_t index=0U; index<injectorChannels.primary; ++index) {
    fuelSchedules[index].channelDegrees = index * spacing;
  }
}

struct callback_pair_t {
  voidVoidCallback start;
  voidVoidCallback end;
};

static inline void setCallbacks_P(Schedule &schedule, const callback_pair_t &cbPair) {
  setCallbacks(schedule, (voidVoidCallback)pgm_read_ptr(&cbPair.start), (voidVoidCallback)pgm_read_ptr(&cbPair.end));
}

struct callback_pack_t {
  const callback_pair_t *pCallbacks;
  uint8_t length;
};

static callback_pack_t getSequentialInjectionCallbacks(void) {
  static const callback_pair_t callbacks[] PROGMEM = {
    { openInjectorT<1>, closeInjectorT<1> },
    { openInjectorT<2>, closeInjectorT<2> },
    { openInjectorT<3>, closeInjectorT<3> },
    { openInjectorT<4>, closeInjectorT<4> },
    { openInjectorT<5>, closeInjectorT<5> },
    { openInjectorT<6>, closeInjectorT<6> },
    { openInjectorT<7>, closeInjectorT<7> },
    { openInjectorT<8>, closeInjectorT<8> },
  };
  static_assert((size_t)INJ_CHANNELS<=_countof(callbacks), "You need to define more sequential callbacks.");
  return { callbacks, _countof(callbacks) };
}

static callback_pack_t getPairedInjectionCallbacks(void)  {
  callback_pack_t cb = getSequentialInjectionCallbacks();
  cb.length = min(cb.length, (uint8_t)INJ_CHANNELS);
  return cb;
}

static callback_pack_t getSemiSequentialInjectionCallbacks(void)  {
  //Semi-Sequential injection. Currently possible with 4, 6 and 8 cylinders. 5 cylinder is a special case
  if( configPage2.nCylinders == 4U )
  {
    if(configPage4.inj4cylPairing == INJ_PAIR_13_24) {
      static const callback_pair_t callbacks[] PROGMEM = {
        { compoundCallback<openInjectorT<1>, openInjectorT<3>>, compoundCallback<closeInjectorT<1>, closeInjectorT<3>> },
        { compoundCallback<openInjectorT<2>, openInjectorT<4>>, compoundCallback<closeInjectorT<2>, closeInjectorT<4>> },
      };
      return { callbacks, _countof(callbacks) };
    }

    static const callback_pair_t callbacks[] PROGMEM = {
      { compoundCallback<openInjectorT<1>, openInjectorT<4>>, compoundCallback<closeInjectorT<1>, closeInjectorT<4>> },
      { compoundCallback<openInjectorT<2>, openInjectorT<3>>, compoundCallback<closeInjectorT<2>, closeInjectorT<3>> },
    };
    return { callbacks, _countof(callbacks) };
  }

  if( configPage2.nCylinders == 5U ) { //This is similar to the paired injection but uses five injector outputs instead of four
    static const callback_pair_t callbacks[] PROGMEM = {
      { openInjectorT<1>, closeInjectorT<1> },
      { openInjectorT<2>, closeInjectorT<2> },
      { compoundCallback<openInjectorT<3>, openInjectorT<5>>, compoundCallback<closeInjectorT<3>, closeInjectorT<5>> },
      { openInjectorT<4>, closeInjectorT<4> },
    };
    return { callbacks, _countof(callbacks) };
  }

  if( configPage2.nCylinders == 6U ) {
    static const callback_pair_t callbacks[] PROGMEM = {
      { compoundCallback<openInjectorT<1>, openInjectorT<4>>, compoundCallback<closeInjectorT<1>, closeInjectorT<4>> },
      { compoundCallback<openInjectorT<2>, openInjectorT<5>>, compoundCallback<closeInjectorT<2>, closeInjectorT<5>> },
      { compoundCallback<openInjectorT<3>, openInjectorT<6>>, compoundCallback<closeInjectorT<3>, closeInjectorT<6>> },
    };
    return { callbacks, _countof(callbacks) };
  }

  if( configPage2.nCylinders == 8U ) {
    static const callback_pair_t callbacks[] PROGMEM = {
      { compoundCallback<openInjectorT<1>, openInjectorT<5>>, compoundCallback<closeInjectorT<1>, closeInjectorT<5>> },
      { compoundCallback<openInjectorT<2>, openInjectorT<6>>, compoundCallback<closeInjectorT<2>, closeInjectorT<6>> },
      { compoundCallback<openInjectorT<3>, openInjectorT<7>>, compoundCallback<closeInjectorT<3>, closeInjectorT<7>> },
      { compoundCallback<openInjectorT<4>, openInjectorT<8>>, compoundCallback<closeInjectorT<4>, closeInjectorT<8>> },
    };
    return { callbacks, _countof(callbacks) };
  }
  
  return getPairedInjectionCallbacks();
}

static callback_pack_t getFuelScheduleCallbackPack(uint8_t injLayout)
{
  switch(injLayout)
  {
  case INJ_SEMISEQUENTIAL:
      return getSemiSequentialInjectionCallbacks();
      break;

  case INJ_SEQUENTIAL:
      return getSequentialInjectionCallbacks(); 
      break;

  case INJ_PAIRED: // Paired injection
  default:
      return getPairedInjectionCallbacks();
      break;
  }
}

static void setFuelScheduleCallbacks(uint8_t injLayout) {
  callback_pack_t cb = getFuelScheduleCallbackPack(injLayout);

  for (uint8_t index = 0U; index<min((uint8_t)_countof(fuelSchedules), cb.length); ++index) {
    setCallbacks_P(fuelSchedules[index], cb.pCallbacks[index]);
  }
  // Set any remaining to null.
  for (uint8_t index = min((uint8_t)_countof(fuelSchedules), cb.length); index<_countof(fuelSchedules); ++index) {
    setCallbacks(fuelSchedules[index], nullCallback, nullCallback);
  }
}

static void initialiseFuelContext(void) 
{
  // Sequential only applies to 4 stroke with enough channels.
  // If those conditions aren't met, revert to paired injection.
  if (configPage2.injLayout == INJ_SEQUENTIAL) {
    if ((configPage2.strokes != FOUR_STROKE) || (configPage2.nCylinders>_countof(fuelSchedules))) {
      configPage2.injLayout = INJ_PAIRED;
    }
  }

  if (configPage2.injLayout == INJ_SEMISEQUENTIAL) {
    // Semi-sequential only valid for 4,5,6,8 cylinders
    // If those conditions aren't met, revert to paired injection.
    if (!(configPage2.nCylinders==4U || configPage2.nCylinders==5U || configPage2.nCylinders==6U || configPage2.nCylinders==8U)) {
      configPage2.injLayout = INJ_PAIRED;
    }
  }
  
  // Fuel trims are only applied in sequential mode.
  configPage6.fuelTrimEnabled = configPage6.fuelTrimEnabled && configPage2.injLayout == INJ_SEQUENTIAL;

  if (configPage2.injLayout == INJ_SEQUENTIAL)
  {
    currentStatus.nSquirts = 1U;
    // Force injection timing when sequential
    configPage2.injTiming = INJ_TIMING_ALTERNATING;
  }
  else
  {
    // nSquirts is complicated.
    if(  (configPage2.nCylinders==3U) 
      && (configPage2.injType == INJ_TYPE_PORT)) {
      //Force nSquirts to 2 for individual port injection. This prevents TunerStudio forcing the value to 3 even when this isn't wanted. 
      currentStatus.nSquirts = 2U;
    } else if (configPage2.divider!=0U) {
      currentStatus.nSquirts = (uint8_t)max((uint8_t)1U, (uint8_t)(configPage2.nCylinders / configPage2.divider));
    } else {
      currentStatus.nSquirts = 2U;
    }    

    if(configPage2.strokes == FOUR_STROKE)
    {
      //Default is 1 squirt per revolution, so we halve the given req-fuel figure (Which would be over 2 revolutions)
      req_fuel_uS = req_fuel_uS / 2U; //The req_fuel calculation above gives the total required fuel (At VE 100%) in the full cycle. If we're doing more than 1 squirt per cycle then we need to split the amount accordingly. (Note that in a non-sequential 4-stroke setup you cannot have less than 2 squirts as you cannot determine the stroke to make the single squirt on)
    }
  }

  CRANK_ANGLE_MAX_INJ = (configPage2.strokes == FOUR_STROKE ? 720U : 360U) / currentStatus.nSquirts;

  currentStatus.status3 |= currentStatus.nSquirts << BIT_STATUS3_NSQUIRTS1; //Top 3 bits of the status3 variable are the number of squirts. This must be done after the above section due to nSquirts being forced to 1 for sequential
}

void initialiseFuelSchedulers(void)
{
  turnOffInjectors();

  resetFuelSchedulers();
  initialiseFuelContext();
  setFuelChannelAngles();
  initialiseStagedInjection();
  setFuelScheduleCallbacks(configPage2.injLayout);
  startFuelSchedulers();
}

TESTABLE_INLINE_STATIC void startIgnitionSchedulers(void)
{
  IGN1_TIMER_ENABLE();
#if IGN_CHANNELS >= 2
  IGN2_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 3
  IGN3_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 4
  IGN4_TIMER_ENABLE();
#endif
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
  for (uint8_t index=0; index<_countof(ignitionSchedules); ++index) {
    endCoilCharge(index+1U);
  }
}

static void setIgnitionChannelAngles(void)
{
  // Rotary has it's own rules
  if (configPage4.sparkMode == IGN_MODE_ROTARY)
  {
    maxIgnOutputs = 4;
    //Rotary uses the ign 3 and 4 schedules for the trailing spark. They are offset from the ign 1 and 2 channels respectively and so use the same degrees as them
    ignitionSchedules[0].channelDegrees = 0;
    ignitionSchedules[1].channelDegrees = 180;
    ignitionSchedules[2].channelDegrees = 0;
    ignitionSchedules[3].channelDegrees = 180;

    configPage4.IgInv = GOING_LOW; //Force Going Low ignition mode (Going high is never used for rotary)
  }
  else if (configPage2.engineType==ODD_FIRE) {
    maxIgnOutputs = configPage2.nCylinders;
    ignitionSchedules[0].channelDegrees = 0;
    for (uint8_t index=1; index<maxIgnOutputs; ++index) {
      ignitionSchedules[index].channelDegrees = configPage2.oddfire[index-1U];
    }
  }
  else
  {
    if (configPage2.nCylinders==4U || configPage2.nCylinders==6U || configPage2.nCylinders==8U) {
      maxIgnOutputs = (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) ? configPage2.nCylinders : configPage2.nCylinders/2U;
    } else {
      // 1, 2, 3 & 5 cylinder are essentially sequential if we have enough channels
      maxIgnOutputs = min((uint8_t)configPage2.nCylinders, (uint8_t)_countof(ignitionSchedules));
    }

    // 1 cylinder is 0°, 2 cylinder is 180°, everything other channel is spaced evenly between 0 & CRANK_ANGLE_MAX_IGN
    uint16_t spacing = (configPage2.nCylinders==1U) ? 0U : ((configPage2.nCylinders==2U) ? 180U : (uint16_t)CRANK_ANGLE_MAX_IGN/maxIgnOutputs);

    // Set the channel angles
    for (uint8_t index = 0U; index<maxIgnOutputs; ++index) {
      ignitionSchedules[index].channelDegrees = spacing * index;
    }
  }    
}

static callback_pack_t getSingleChannelIgnitionCallbacks(void) {
   // Single channel mode. All ignition pulses are on channel 1
  static const callback_pair_t callbacks[] PROGMEM = {
    { beginCoilChargeT<1>, endCoilChargeT<1> },
    { beginCoilChargeT<1>, endCoilChargeT<1> },
    { beginCoilChargeT<1>, endCoilChargeT<1> },
    { beginCoilChargeT<1>, endCoilChargeT<1> },
    { beginCoilChargeT<1>, endCoilChargeT<1> },
    { beginCoilChargeT<1>, endCoilChargeT<1> },
    { beginCoilChargeT<1>, endCoilChargeT<1> },
    { beginCoilChargeT<1>, endCoilChargeT<1> },
  };
  return { callbacks, _countof(callbacks) };
}


static callback_pack_t getSequentialIgnitionCallbacks(void) {
  static const callback_pair_t callbacks[] PROGMEM = {
    { beginCoilChargeT<1>, endCoilChargeT<1> },
    { beginCoilChargeT<2>, endCoilChargeT<2> },
    { beginCoilChargeT<3>, endCoilChargeT<3> },
    { beginCoilChargeT<4>, endCoilChargeT<4> },
    { beginCoilChargeT<5>, endCoilChargeT<5> },
    { beginCoilChargeT<6>, endCoilChargeT<6> },
    { beginCoilChargeT<7>, endCoilChargeT<7> },
    { beginCoilChargeT<8>, endCoilChargeT<8> },
  };
  static_assert((size_t)IGN_CHANNELS<=_countof(callbacks), "You need to define more sequential callbacks.");
  return { callbacks, _countof(callbacks) };
}

static callback_pack_t getWastedSparkIgnitionCallbacks(void) {
  callback_pack_t cb = getSequentialIgnitionCallbacks();
  cb.length = 5;
  return cb;
}

static callback_pack_t getWastedCopIgnitionCallbacks(void) {
  //Wasted COP mode. Note, most of the boards can only run this for 4-cyl only.
  if( configPage2.nCylinders <= 3U) {
      //1-3 cylinder wasted COP is the same as regular wasted mode
      callback_pack_t cb = getSequentialIgnitionCallbacks();
      cb.length = 3;
      return cb;      
  }

  if( configPage2.nCylinders == 4U ) {
    //Wasted COP mode for 4 cylinders. Ignition channels 1&3 and 2&4 are paired together
    static const callback_pair_t callbacks[] PROGMEM = {
      { compoundCallback<beginCoilChargeT<1>, beginCoilChargeT<3>>, compoundCallback<endCoilChargeT<1>, endCoilChargeT<3>> },
      { compoundCallback<beginCoilChargeT<2>, beginCoilChargeT<4>>, compoundCallback<endCoilChargeT<2>, endCoilChargeT<4>> },
    };
    return { callbacks, _countof(callbacks) };
  }

  if( configPage2.nCylinders == 6U ) {
    //Wasted COP mode for 6 cylinders. Ignition channels 1&4, 2&5 and 3&6 are paired together
    static const callback_pair_t callbacks[] PROGMEM = {
      { compoundCallback<beginCoilChargeT<1>, beginCoilChargeT<4>>, compoundCallback<endCoilChargeT<1>, endCoilChargeT<4>> },
      { compoundCallback<beginCoilChargeT<2>, beginCoilChargeT<5>>, compoundCallback<endCoilChargeT<2>, endCoilChargeT<5>> },
      { compoundCallback<beginCoilChargeT<3>, beginCoilChargeT<6>>, compoundCallback<endCoilChargeT<3>, endCoilChargeT<6>> },
    };
    return { callbacks, _countof(callbacks) };
  }
  
  if( configPage2.nCylinders == 8U ) {
    //Wasted COP mode for 8 cylinders. Ignition channels 1&5, 2&6, 3&7 and 4&8 are paired together
    static const callback_pair_t callbacks[] PROGMEM = {
      { compoundCallback<beginCoilChargeT<1>, beginCoilChargeT<5>>, compoundCallback<endCoilChargeT<1>, endCoilChargeT<5>> },
      { compoundCallback<beginCoilChargeT<2>, beginCoilChargeT<6>>, compoundCallback<endCoilChargeT<2>, endCoilChargeT<6>> },
      { compoundCallback<beginCoilChargeT<3>, beginCoilChargeT<7>>, compoundCallback<endCoilChargeT<3>, endCoilChargeT<7>> },
      { compoundCallback<beginCoilChargeT<4>, beginCoilChargeT<8>>, compoundCallback<endCoilChargeT<4>, endCoilChargeT<8>> },
    };
    return { callbacks, _countof(callbacks) };
  };
  
  //If the person has inadvertently selected this when running more than 4 cylinders or other than 6 cylinders, just use standard Wasted spark mode
  return getWastedSparkIgnitionCallbacks();
}

static callback_pack_t getRotaryIgnitionCallbacks(void) {
  if(configPage10.rotaryType == ROTARY_IGN_FC) {
    //Ignition channel 1 is a wasted spark signal for leading signal on both rotors
    static const callback_pair_t callbacks[] PROGMEM = {
      { beginCoilChargeT<1>, endCoilChargeT<1> },
      { beginCoilChargeT<1>, endCoilChargeT<1> },
      { beginCoilChargeT<2>, compoundCallback<endCoilChargeT<1>, beginCoilChargeT<2>> },
      { beginCoilChargeT<2>, compoundCallback<endCoilChargeT<1>, endCoilChargeT<2>> },
    };
    return { callbacks, _countof(callbacks) };
  }
  
  if(configPage10.rotaryType == ROTARY_IGN_FD) {
    //Ignition channel 1 is a wasted spark signal for leading signal on both rotors
    static const callback_pair_t callbacks[] PROGMEM = {
      { beginCoilChargeT<1>, endCoilChargeT<1> },
      { beginCoilChargeT<1>, endCoilChargeT<1> },
    //Trailing coils have their own channel each
    //IGN2 = front rotor trailing spark
      { beginCoilChargeT<2>, endCoilChargeT<2> },
    //IGN3 = rear rotor trailing spark
      { beginCoilChargeT<3>, endCoilChargeT<3> },
    //IGN4 not used
    };
    return { callbacks, _countof(callbacks) };
  }
  
  if(configPage10.rotaryType == ROTARY_IGN_RX8) {
    //RX8 outputs are simply 1 coil and 1 output per plug
    callback_pack_t cb = getSequentialIgnitionCallbacks();
    cb.length = 4;
    return cb;
  }

  return { nullptr, 0 };
}

static callback_pack_t getIgnitionCallbackPack(uint8_t sparkMode) {
  switch(sparkMode)
  {
  case IGN_MODE_SINGLE:
      return getSingleChannelIgnitionCallbacks();
      break;

  case IGN_MODE_WASTEDCOP:
      return getWastedCopIgnitionCallbacks();
      break;

  case IGN_MODE_SEQUENTIAL:
      return getSequentialIgnitionCallbacks();
      break;

  case IGN_MODE_ROTARY:
      return getRotaryIgnitionCallbacks();
      break;

  case IGN_MODE_WASTED: //Wasted Spark (Normal mode)
  default: //Wasted spark (Shouldn't ever happen anyway)
      return getWastedSparkIgnitionCallbacks();
      break;
  }
}

static void setIgnitionScheduleCallbacks(uint8_t sparkMode)
{
  callback_pack_t cb = getIgnitionCallbackPack(sparkMode);

  for (uint8_t index = 0U; index<min((uint8_t)_countof(ignitionSchedules), cb.length); ++index) {
    setCallbacks_P(ignitionSchedules[index], cb.pCallbacks[index]);
  }
  // Set any remaining to null.
  for (uint8_t index = min((uint8_t)_countof(ignitionSchedules), cb.length); index<_countof(ignitionSchedules); ++index) {
    setCallbacks(ignitionSchedules[index], nullCallback, nullCallback);
  }
}


static void initialiseIgnitionContext(void)
{ 
  // Rotary only works on 4 stroke
  if ((configPage4.sparkMode == IGN_MODE_ROTARY) && (configPage2.nCylinders!=4U)) {
    configPage4.sparkMode = IGN_MODE_WASTED;
  }

  if (configPage2.engineType==ODD_FIRE) {
    // Oddfire only applicable to 2 or more cylinders up to the total number of 
    // user entered angles.  I.e. 1<cylinders<_countof(configPage2.oddfire)+2 
    if ((configPage2.nCylinders==1U) || (configPage2.nCylinders>(_countof(configPage2.oddfire)+1U))) {
      configPage2.engineType = EVEN_FIRE;
    }
  }

  // Sequential only works on 4 stroke, odd_fire & <3 cylinders or even fire with enough channels
  if (configPage4.sparkMode == IGN_MODE_SEQUENTIAL)
  {
    if (configPage2.strokes != FOUR_STROKE) {
      configPage4.sparkMode = IGN_MODE_WASTED;
    } else if ((configPage2.engineType == ODD_FIRE) && (configPage2.nCylinders>2U)) {
      configPage4.sparkMode = IGN_MODE_WASTED;
    } else if (_countof(ignitionSchedules)<configPage2.nCylinders) {
      configPage4.sparkMode = IGN_MODE_WASTED;
    } else {
      // All is well - use sequential.
    }
  }
  
  CRANK_ANGLE_MAX_IGN = configPage4.sparkMode == IGN_MODE_SEQUENTIAL ? 720 : 360;
}

void initialiseIgnitionSchedulers(void)
{
  turnOffCoils();

  resetIgnitionSchedulers();
  initialiseIgnitionContext();
  setIgnitionChannelAngles();
  setIgnitionScheduleCallbacks(configPage4.sparkMode);  
  startIgnitionSchedulers();
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
  if ((configPage4.useDwellLim!=0U) && (isCrankLocked != true)) {
    uint32_t targetOverdwellTime = micros() - configPage4.dwellLimit * 1000U;

    for (uint8_t index=0U; index<maxIgnOutputs; ++index) {
      applyChannelOverDwellProtection(ignitionSchedules[index], targetOverdwellTime);
    }
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
    primingValue = primingValue * 100U * 5U; //to achieve long enough priming pulses, the values in tuner studio are divided by 0.5 instead of 0.1, so multiplier of 5 is required.
    for (uint8_t index=0U; index<totalInjectorChannels(injectorChannels); ++index) {
      _setSchedule(fuelSchedules[index], 100U, primingValue); 
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

/** Change injectors angles to 720deg.
 * Roll back req_fuel size and set number of outputs equal to cylinder count.
* */
static void changeInjectionHalfToFullSync(void)
{
  if (CRANK_ANGLE_MAX_INJ != 720) {
    ATOMIC() {
      if (!isAnyFuelScheduleRunning()) {
        CRANK_ANGLE_MAX_INJ = 720;
        injectorChannels.primary = injectorChannels.primary * 2U;
        injectorChannels.secondary = injectorChannels.secondary * 2U;
        req_fuel_uS *= 2U;
        setFuelScheduleCallbacks(INJ_SEQUENTIAL);
      }
    }
  }
}

/** Change injectors or/and ignition angles to 360deg.
 * In semi sequentiol mode req_fuel size is half.
 * Set number of outputs equal to half cylinder count.
* */
static void changeInjectionFullToHalfSync(void)
{
  if(CRANK_ANGLE_MAX_INJ!=360) {
    ATOMIC() {
        CRANK_ANGLE_MAX_INJ = 360;
        injectorChannels.primary = injectorChannels.primary / 2U;
        injectorChannels.secondary = injectorChannels.secondary / 2U;
        req_fuel_uS = req_fuel_uS / 2U;
        setFuelScheduleCallbacks(INJ_SEMISEQUENTIAL);
      }
  }
}

void matchInjectionModeToSyncStatus(void) {
  if (configPage2.injLayout == INJ_SEQUENTIAL) {
    if (currentStatus.hasSync==1) {
      changeInjectionHalfToFullSync();
    } else if (BIT_CHECK(currentStatus.status3, BIT_STATUS3_HALFSYNC)) {
      changeInjectionFullToHalfSync();
    } else {
      // Nothing to do but keep the MISRA checker happy
    }
  }
}

static void changeIgnitionFullToHalfSync(void) {
  if (CRANK_ANGLE_MAX_IGN != 360)  {
    ATOMIC() {
      if (!isAnyIgnScheduleRunning()) {
        CRANK_ANGLE_MAX_IGN = 360;
        maxIgnOutputs = configPage2.nCylinders / 2U;

        setIgnitionScheduleCallbacks(IGN_MODE_WASTEDCOP);
      }
    }
  }
}

static void changeIgnitionHalfToFullSync(void) {
  if (CRANK_ANGLE_MAX_IGN != 720) {
    ATOMIC() {
      CRANK_ANGLE_MAX_IGN = 720;
      maxIgnOutputs = maxIgnOutputs * 2U;
      setIgnitionScheduleCallbacks(IGN_MODE_SEQUENTIAL);
    }
  }
}

void matchIgnitionModeToSyncStatus(void) {
  if (configPage4.sparkMode==IGN_MODE_SEQUENTIAL) {
    if (currentStatus.hasSync==1) {
      changeIgnitionHalfToFullSync();
    } else if (BIT_CHECK(currentStatus.status3, BIT_STATUS3_HALFSYNC)) {
      changeIgnitionFullToHalfSync();    
    } else {
      // Nothing to do but keep the MISRA checker happy
    }
  }  
}
