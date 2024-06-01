/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/** @file
Corrections to injection pulsewidth.
The corrections functions in this file affect the fuel pulsewidth (Either increasing or decreasing)
based on factors other than the VE lookup.

These factors include:
- Temperature (Warmup Enrichment and After Start Enrichment)
- Acceleration/Deceleration
- Flood clear mode
- etc.

Most correction functions return value 100 (like 100% == 1) for no need for correction.

There are 2 top level functions that call more detailed corrections for Fuel and Ignition respectively:
- @ref correctionsFuel() - All fuel related corrections
- @ref correctionsIgn() - All ignition related corrections
*/
//************************************************************************************************************

#include "globals.h"
#include "corrections.h"
#include "speeduino.h"
#include "timers.h"
#include "maths.h"
#include "sensors.h"
#include "src/PID_v1/PID_v1.h"
#include "unit_testing.h"
#include "scale_translate.h"
#include "idle.h"

static long PID_O2;
static long PID_output;
static long PID_AFRTarget;
/** Instance of the PID object in case that algorithm is used (Always instantiated).
* Needs to be global as it maintains state outside of each function call.
* Comes from Arduino (?) PID library.
*/
static PID egoPID(&PID_O2, &PID_output, &PID_AFRTarget, configPage6.egoKP, configPage6.egoKI, configPage6.egoKD, REVERSE);

static uint16_t aeActivatedReading; //The mapDOT/tpsDOT value seen when the MAE/TAE was activated. 

TESTABLE_STATIC uint16_t AFRnextCycle;
static unsigned long knockStartTime;
static uint8_t knockLastRecoveryStep;
//static int16_t knockWindowMin; //The current minimum crank angle for a knock pulse to be valid
//static int16_t knockWindowMax;//The current maximum crank angle for a knock pulse to be valid
static uint8_t dfcoTaper;

// Constant that represents "no fuel correction"
static constexpr uint8_t NO_FUEL_CORRECTION = ONE_HUNDRED_PCT;
// Constant that represents the baseline fuel correction to be modified
// (yes, it's the same as NO_FUEL_CORRECTION, but captures a slightly different concept)
static constexpr uint8_t BASELINE_FUEL_CORRECTION = ONE_HUNDRED_PCT;


/** Initialise instances and vars related to corrections (at ECU boot-up).
 */
void initialiseCorrections(void)
{
  PID_output = 0L;
  PID_O2 = 0L;
  PID_AFRTarget = 0L;
  // Toggling between modes resets the PID internal state
  // This is required by the unit tests
  // TODO: modify PID code to provide a method to reset it. 
  egoPID.SetMode(AUTOMATIC);
  egoPID.SetMode(MANUAL);
  egoPID.SetMode(AUTOMATIC);

  currentStatus.flexIgnCorrection = 0;
  //Default value of no adjustment must be set to avoid randomness on first correction cycle after startup
  currentStatus.egoCorrection = NO_FUEL_CORRECTION; 
  currentStatus.ASEValue = NO_FUEL_CORRECTION;
  currentStatus.wueCorrection = NO_FUEL_CORRECTION;
  currentStatus.iatCorrection = NO_FUEL_CORRECTION;
  currentStatus.baroCorrection = NO_FUEL_CORRECTION;
  currentStatus.batCorrection = NO_FUEL_CORRECTION;
  AFRnextCycle = 0;
  BIT_CLEAR(currentStatus.status5, BIT_STATUS5_KNOCK_ACTIVE);
  BIT_CLEAR(currentStatus.status5, BIT_STATUS5_KNOCK_PULSE);
  currentStatus.knockCount = 1;
  knockLastRecoveryStep = 0;
  knockStartTime = 0;
  currentStatus.battery10 = 125; //Set battery voltage to sensible value for dwell correction for "flying start" (else ignition gets spurious pulses after boot)  

  BIT_CLEAR(currentStatus.status2, BIT_STATUS2_HLAUNCH);
  BIT_CLEAR(currentStatus.status2, BIT_STATUS2_SLAUNCH);
}

// ============================= Warm Up Enrichment =============================

/** Warm Up Enrichment (WUE) corrections.
Uses a 2D enrichment table (WUETable) where the X axis is engine temp and the Y axis is the amount of extra fuel to add
*/
TESTABLE_INLINE_STATIC uint8_t correctionWUE(statuses &current, const table2D &lookupTable)
{
  uint8_t accelCorrection = current.wueCorrection;

  // Only update as fast as the sensor is read
  if( BIT_CHECK(LOOP_TIMER, CLT_READ_TIMER_BIT) ) { 
    if (current.coolant >= toWorkingTemperature((uint8_t)table2D_getAxisValue(&lookupTable, lookupTable.xSize-1U)))
    {
      //This prevents us doing the 2D lookup if we're already up to temp
      BIT_CLEAR(current.engine, BIT_ENGINE_WARMUP);
      accelCorrection = (uint8_t)table2D_getRawValue(&lookupTable, lookupTable.xSize-1U);
    }
    else
    {
      BIT_SET(current.engine, BIT_ENGINE_WARMUP);
      accelCorrection = (uint8_t)table2D_getValue(&lookupTable, toStorageTemperature(current.coolant));
    }
  }

  return accelCorrection;
}

// ============================= Cranking Enrichment =============================

/** Cranking Enrichment corrections.
Additional fuel % to be added when the engine is cranking
*/

static inline uint16_t lookUpCrankingEnrichmentPct(const statuses &current, const table2D &lookupTable) {
  return  toWorkingU8U16( CRANKING_ENRICHMENT, 
                          (uint8_t)table2D_getValue(&lookupTable, toStorageTemperature(current.coolant)));
}

//Taper start value needs to account for ASE that is now running, so total correction does not increase when taper begins
static inline uint16_t computeCrankingTaperStartPct(uint16_t crankingPercent, const statuses &current) {
  // Avoid 32-bit division if possible
  if (BIT_CHECK(current.engine, BIT_ENGINE_ASE) && (current.ASEValue!=NO_FUEL_CORRECTION)) {
    return udiv_32_16((uint32_t)crankingPercent * BASELINE_FUEL_CORRECTION, current.ASEValue);
  }

  return crankingPercent;
}

TESTABLE_INLINE_STATIC uint16_t correctionCranking(const statuses &current, const table2D &lookupTable, const config10 &page10)
{
  static uint8_t crankingEnrichTaper = 0U;

  uint16_t accelCorrection = NO_FUEL_CORRECTION;

  //Check if we are actually cranking
  if ( BIT_CHECK(current.engine, BIT_ENGINE_CRANK) )
  {
    accelCorrection = lookUpCrankingEnrichmentPct(current, lookupTable);
    crankingEnrichTaper = 0U;
  }
  //If we're not cranking, check if if cranking enrichment tapering to ASE should be done
  else if ( crankingEnrichTaper < page10.crankingEnrichTaper )
  {
    accelCorrection = (uint16_t) map( crankingEnrichTaper, 
                                      0U, page10.crankingEnrichTaper, 
                                      computeCrankingTaperStartPct(lookUpCrankingEnrichmentPct(current, lookupTable), current), NO_FUEL_CORRECTION); //Taper from start value to 100%
    if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { ++crankingEnrichTaper; }
  } else {
    // Not cranking and taper not in effect, so no cranking enrichment needed.
    // just need to keep MISRA checker happy.
  }

  return max((uint16_t)NO_FUEL_CORRECTION, (uint16_t)accelCorrection);
}

// ============================= After Start Enrichment =============================

/** After Start Enrichment calculation.
 * This is a short period (Usually <20 seconds) immediately after the engine first fires (But not when cranking)
 * where an additional amount of fuel is added (Over and above the WUE amount).
 * 
 * @return uint8_t The After Start Enrichment modifier as a %. 100% = No modification. 
 */   
TESTABLE_INLINE_STATIC uint8_t correctionASE(statuses &current, const table2D &durationTable, const table2D &amountTable, const config2 &page2)
{
  // We use aseTaper both to track taper AND as a flag value
  // to tell when ASE is complete and avoid unecessary table lookups.
  constexpr uint8_t ASE_COMPLETE = UINT8_MAX;
  static uint8_t aseTaper = 0U;

  uint8_t ASEValue = NO_FUEL_CORRECTION;

  if (BIT_CHECK(current.engine, BIT_ENGINE_CRANK)) {
    // Engine is cranking - mark ASE as inactive and ready to run 
    BIT_CLEAR(current.engine, BIT_ENGINE_ASE); 
    aseTaper = 0U; 
    ASEValue = NO_FUEL_CORRECTION;
  } else if (aseTaper!=ASE_COMPLETE) {
    // ASE hasn't started or isn't complete.
    if ( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ))
    {
      // We only update ASE every 100ms for performance reasons - coolant
      // doesn't change temperature that quickly. 
      //
      // We must use 100ms (rather than CLT_READ_TIMER_BIT) since aseTaper counts tenths of a second.
      
      if ((aseTaper==0U) // Avoid table lookup if taper is being applied
       && (current.runSecs < ((uint8_t)table2D_getValue(&durationTable, toStorageTemperature(current.coolant)))) )
      {
        BIT_SET(current.engine, BIT_ENGINE_ASE);
        ASEValue = BASELINE_FUEL_CORRECTION + (uint8_t)table2D_getValue(&amountTable, toStorageTemperature(current.coolant));
      } else if ( aseTaper < page2.aseTaperTime ) { //Check if we've reached the end of the taper time
        BIT_SET(current.engine, BIT_ENGINE_ASE);
        ASEValue = BASELINE_FUEL_CORRECTION + (uint8_t)map(aseTaper, 
                                        0U, page2.aseTaperTime, 
                                        (uint8_t)table2D_getValue(&amountTable, toStorageTemperature(current.coolant)), 0);
        aseTaper = aseTaper + 1U;
      } else {
        // ASE has finished
        BIT_CLEAR(current.engine, BIT_ENGINE_ASE); //Mark ASE as inactive.
        aseTaper = ASE_COMPLETE; // Flag ASE as complete
        ASEValue = NO_FUEL_CORRECTION;
      }
    } else {
      // ASE is in effect, but we're not due to update, so reuse previous value.
      ASEValue = current.ASEValue;
    }    
  } else {
    // ASE is finished, nothing to do but keep MISRA checker happy 
  }

  return ASEValue;
}

// ============================= Acceleration Enrichment =============================

static inline void accelEnrichmentOff(statuses &current) {
  BIT_CLEAR(current.engine, BIT_ENGINE_ACC);
  BIT_CLEAR(current.engine, BIT_ENGINE_DCC);
  current.AEamount = 0;
}

static inline bool isAccelEnrichmentOn(const statuses &current) {
  return (BIT_CHECK(current.engine, BIT_ENGINE_ACC)) || (BIT_CHECK(current.engine, BIT_ENGINE_DCC));
}

static inline uint8_t applyAeRpmTaper(uint8_t accelCorrection, const statuses &current, const config2 &page2) {
  //Apply the RPM taper to the above
  //The RPM settings are stored divided by 100:
  if ((page2.aeTaperMax>page2.aeTaperMin) && (accelCorrection>0U)) {
    const uint16_t taperMinRpm = toWorkingU8U16(RPM_COARSE, page2.aeTaperMin);
    // If RPM is lower than the taper range, no correction 
    if (current.RPM > taperMinRpm)
    {
      const uint16_t taperMaxRpm = toWorkingU8U16(RPM_COARSE, page2.aeTaperMax);
      if(current.RPM > taperMaxRpm) { 
        // RPM is above taper range, so accel enrich is turned off
        accelCorrection = 0U;
      } else {
        // RPM is within the taper range, compute the *reverse* percentage
        // of it's position within the RPM taper range
        const auto taperPercent = (uint8_t)map( current.RPM,
                                                  taperMinRpm, taperMaxRpm,
                                                  ONE_HUNDRED_PCT, 0U); 
        accelCorrection = (uint8_t)percentage(taperPercent, accelCorrection); //Calculate the above percentage of the calculated accel amount. 
      }
    }
  }

  return accelCorrection;
}

static inline uint16_t applyAeCoolantTaper(uint16_t accelCorrection, const statuses &current, const config2 &page2) {
  //Apply AE cold coolant modifier, if CLT is less than taper end temperature
  if ( (accelCorrection!=0U)
    && (page2.aeColdPct!=ONE_HUNDRED_PCT)
    && (page2.aeColdTaperMax>page2.aeColdTaperMin)
    && (current.coolant < toWorkingTemperature(page2.aeColdTaperMax) ))
  {
    //If CLT is less than taper min temp, apply full modifier on top of accelCorrection
    if ( current.coolant <= toWorkingTemperature(page2.aeColdTaperMin) )
    {
      accelCorrection = (uint16_t)percentage(page2.aeColdPct, accelCorrection);
    }
    //If CLT is between taper min and max, taper the modifier value and apply it on top of accelCorrection
    else
    {
      // Tune uses 100% as no adjustment, range 100% to 255%. So subtract 100 to get the adjustment, 
      // scale the adjustment over the coolant RPM range & reapply the 100 offset 
      const uint8_t coldPct = ONE_HUNDRED_PCT + (uint8_t)map( toStorageTemperature(current.coolant),
                                                              page2.aeColdTaperMin, page2.aeColdTaperMax,
                                                              page2.aeColdPct-ONE_HUNDRED_PCT, 0U);       
      accelCorrection = (uint16_t)percentage(coldPct, accelCorrection);
    }
  }

  return accelCorrection;
}

static inline uint16_t calcAccelEnrichment(const uint8_t accelCorrection, statuses &current, const config2 &page2) {
  BIT_SET(current.engine, BIT_ENGINE_ACC); //Mark acceleration enrichment as active.
  return BASELINE_FUEL_CORRECTION + applyAeCoolantTaper(applyAeRpmTaper(accelCorrection, current, page2), current, page2);
}

static inline uint16_t calcDeccelEnrichment(statuses &current, const config2 &page2) {
  BIT_SET(current.engine, BIT_ENGINE_DCC); //Mark deceleration enleanment as active.
  return page2.decelAmount; //In decel, use the decel fuel amount as accelCorrection
}

static inline bool aeTimeoutExpired(const statuses &current) {
  return micros() >= current.AEEndTime;
}

//Set the time in the future where the enrichment will be turned off. 
static inline void updateAeTimeout(statuses &current, const config2 &page2) {
  // taeTime is stored as mS / 10, so multiply it by 10000 to get it in uS
  current.AEEndTime = micros() + toWorkingU32(TIME_TENTH_MILLIS, page2.aeTime); 
}

using aeTimeoutExpiredCallback_t = void (*)(statuses &);
using shouldResetCurrentAeCallback_t = bool (*)(const statuses &);
using shouldStartAeCallback_t = bool (*)(const statuses &, const config2 &);
using computAeCallback_t = uint16_t (*)(statuses &, const config2 &, const table2D &);

// Implements the skeleton of the AE algorithm. Callers fill in specific steps via callbacks
// (Template Method design pattern in C!)
static inline uint16_t correctionAccel( const aeTimeoutExpiredCallback_t onTimeoutExpired, 
                                        const shouldResetCurrentAeCallback_t shouldResetCurrentAe, 
                                        const shouldStartAeCallback_t shouldStartAe, 
                                        const computAeCallback_t computeAe,
                                        statuses &current, 
                                        const config2 &page2,
                                        const table2D &lookupTable) {
  uint16_t accelCorrection = NO_FUEL_CORRECTION;

  //First, check whether the accel. enrichment is already running
  if (isAccelEnrichmentOn(current)) {
    //If it is currently running, check whether it should still be running or whether it's reached it's end time
    if (aeTimeoutExpired(current)) {
      accelEnrichmentOff(current);
      // Timed out, reset
      onTimeoutExpired(current);
    //Need to check whether the accel amount has increased from when AE was turned on
    //If the accel amount HAS increased, we clear the current enrich phase and a new one will be started below
     } else if(shouldResetCurrentAe(current)) {
        accelEnrichmentOff(current);
    } else {
      //Enrichment still needs to keep running. 
      //Simply return the current amount
      accelCorrection = current.AEamount;
    }
  }

  //Need to check this again as it may have been changed in the above section (Both ACC and DCC are off if this has changed)
  if ((!isAccelEnrichmentOn(current)) && (shouldStartAe(current, page2))) {
    updateAeTimeout(current, page2);
    accelCorrection = computeAe(current, page2, lookupTable);
  } 

  return accelCorrection;
}

static inline void mapOnTimeoutExpired(statuses &current) { 
  current.mapDOT = 0; 
}

static inline bool mapShouldResetAe(const statuses &current) {
  return (uint16_t)abs(current.mapDOT) > aeActivatedReading; 
}

static inline bool mapShouldStartAe(const statuses &current, const config2 &page2) { 
  return (uint16_t)abs(current.mapDOT) > page2.maeThresh; 
};

static inline uint16_t mapComputeAe(statuses &current, const config2 &page2, const table2D &lookupTable) {
  uint16_t aeEnrichment = 0U;

  if (current.mapDOT < 0) {
    aeEnrichment = calcDeccelEnrichment(current, page2);
  } else {
    aeEnrichment = calcAccelEnrichment((uint8_t)table2D_getValue(&lookupTable, toRawU8(MAP_DOT, current.mapDOT)), current, page2);
  } 
  
  aeActivatedReading = (uint16_t)abs(current.mapDOT);
  
  return aeEnrichment;
}

static inline int16_t computeMapDot(const statuses &current, const config2 &page2) {
  int16_t mapDOT = 0U;
  const int16_t mapChange = (int16_t)current.MAP - (int16_t)MAPlast;
  // Check for only very small movement. This not only means we can skip the lookup, but helps reduce false triggering around 0-2% throttle openings
  if ( ((uint16_t)abs(mapChange) > page2.maeMinChange)
    // Sanity check
    && (MAP_time>MAPlast_time)) {
    const uint32_t mapDeltaT = MAP_time - MAPlast_time;

    static constexpr uint32_t MAX_udiv_32_16 = UINT16_MAX; 
    static constexpr uint32_t MIN_udiv_32_16 = (MICROS_PER_SEC/UINT16_MAX)+1U; 
    // Faster division path. Will almost always be taken when above idle - a 360° cycle time of 65535µS
    // equals 915 RPM.
    if ((mapDeltaT<=MAX_udiv_32_16) && (mapDeltaT>MIN_udiv_32_16)) {
      mapDOT = (int16_t)udiv_32_16(MICROS_PER_SEC, mapDeltaT) * (int16_t)mapChange; 
    } else {
      mapDOT = (int16_t)(MICROS_PER_SEC / mapDeltaT) * mapChange;
    }

    static constexpr int16_t MAP_DOT_MIN = -2550;
    static constexpr int16_t MAP_DOT_MAX = 2550;
    mapDOT = constrain(mapDOT, MAP_DOT_MIN, MAP_DOT_MAX);
  }
  return mapDOT;
}

static inline uint16_t correctionAccelModeMap(statuses &current, const config2 &page2, const table2D &lookupTable) {
  uint16_t aeCorrection = current.AEamount;

  // No point in updating faster than the MAP sensor is read
  if (BIT_CHECK(LOOP_TIMER, MAP_READ_TIMER_BIT)) {
    current.mapDOT = computeMapDot(current, page2);

    aeCorrection = correctionAccel( mapOnTimeoutExpired, mapShouldResetAe, mapShouldStartAe, mapComputeAe,
                                    current, page2, lookupTable);
  }

  return aeCorrection;
}

static inline void tpsOnTimeoutExpired(statuses &current) { 
  current.tpsDOT = 0; 
}

static inline bool tpsShouldResetAe(const statuses &current) { 
  return (uint16_t)abs(current.tpsDOT) > aeActivatedReading; 
}

static inline bool tpsShouldStartAe(const statuses &current, const config2 &page2) { 
  return (uint16_t)abs(current.tpsDOT) > page2.taeThresh;
}

static inline uint16_t tpsComputeAe(statuses &current, const config2 &page2, const table2D &lookupTable) {
  uint16_t aeEnrichment = 0U;

  //Check if the TPS rate of change is negative or positive. Negative means decelaration.
  if (current.tpsDOT < 0) {
    aeEnrichment = calcDeccelEnrichment(current, page2);
  } else {
    aeEnrichment = calcAccelEnrichment((uint8_t)table2D_getValue(&lookupTable, toRawU8(TPS_DOT, current.tpsDOT)), current, page2); 
  }
  aeActivatedReading = (uint16_t)abs(current.tpsDOT);

  return aeEnrichment;
}

static inline int16_t computeTPSDOT(const statuses &current, const config2 &page2) {
  //Get the TPS rate change
  const int16_t tpsChange = (int16_t)current.TPS - (int16_t)current.TPSlast;
  
  int16_t tpsDOT = 0;
  // Check for only very small movement. This not only means we can skip the lookup, but helps reduce false triggering around 0-2% throttle openings
  if ((uint16_t)abs(tpsChange) > page2.taeMinChange) {
    tpsDOT = (TPS_READ_FREQUENCY * tpsChange) / 2; //This is the % per second that the TPS has moved, adjusted for the 0.5% resolution of the TPS

    static constexpr int16_t TPS_DOT_MIN = -2550;
    static constexpr int16_t TPS_DOT_MAX = 2550;
    tpsDOT = constrain(tpsDOT, TPS_DOT_MIN, TPS_DOT_MAX);
  } 
  return tpsDOT;
}

static inline uint16_t correctionAccelModeTps(statuses &current, const config2 &page2, const table2D &lookupTable) {
  uint16_t aeCorrection = current.AEamount;

  // No point in updating faster than the TPS is read
  if (BIT_CHECK(LOOP_TIMER, TPS_READ_TIMER_BIT)) {
    current.tpsDOT = computeTPSDOT(current, page2);

    aeCorrection = correctionAccel(tpsOnTimeoutExpired, tpsShouldResetAe, tpsShouldStartAe, tpsComputeAe,
                                    current, page2, lookupTable);
  }

  return aeCorrection;
}

/** Acceleration enrichment correction calculation.
 * 
 * Calculates the % change of the throttle over time (%/second) and performs a lookup based on this
 * Coolant-based modifier is applied on the top of this.
 * When the enrichment is turned on, it runs at that amount for a fixed period of time (taeTime)
 * 
 * @return uint16_t The Acceleration enrichment modifier as a %. 100% = No modification.
 * 
 * As the maximum enrichment amount is +255% and maximum cold adjustment for this is 255%, the overall return value
 * from this function can be 100+(255*255/100)=750. Hence this function returns a uint16_t rather than uint8_t.
 */
TESTABLE_INLINE_STATIC uint16_t correctionAccel(statuses &current, const config2 &page2, const table2D &tpsLookupTable, const table2D &mapLookupTable)
{
  if(AE_MODE_MAP==page2.aeMode) {
    return correctionAccelModeMap(current, page2, mapLookupTable);
  }
  if(AE_MODE_TPS==page2.aeMode) {
    return correctionAccelModeTps(current, page2, tpsLookupTable);
  }
  return NO_FUEL_CORRECTION;
}

// ============================= Flood Clear =============================

static inline bool isFloodClearActive(const statuses &current, const config4 &page4) {
  return (BIT_CHECK(current.engine, BIT_ENGINE_CRANK))
      && (current.TPS >= page4.floodClear);
}

/** Simple check to see whether we are cranking with the TPS above the flood clear threshold.
@return 100 (not cranking and thus no need for flood-clear) or 0 (Engine cranking and TPS above @ref config4.floodClear limit).
*/
TESTABLE_INLINE_STATIC uint8_t correctionFloodClear(const statuses &current, const config4 &page4)
{
  return isFloodClearActive(current, page4) ? 0U : NO_FUEL_CORRECTION;
}

// ============================= Battery Voltage =============================

/** Battery Voltage correction.
Uses a 2D enrichment table (injectorVCorrectionTable) where the X axis is battery voltage and the Y axis is the percent of extra fuel to add.
*/
TESTABLE_INLINE_STATIC uint8_t correctionBatVoltage(const statuses &current, const table2D &lookupTable, const config2 &page2)
{
  // No point in updating more often than the sensor is read
  uint8_t correction = current.batCorrection;
  if( BIT_CHECK(LOOP_TIMER, BAT_READ_TIMER_BIT) ) { 
    correction = (uint8_t)table2D_getValue(&lookupTable, current.battery10);
  }
  
  if (page2.battVCorMode == BATTV_COR_MODE_OPENTIME) {
    inj_opentime_uS = page2.injOpen * correction; // Apply voltage correction to injector open time.
    correction = NO_FUEL_CORRECTION; // This is to ensure that the correction is not applied twice. There is no battery correction fator as we have instead changed the open time
  }
  return correction;
}

// ============================= IAT correction =============================

/** Simple temperature based corrections lookup based on the inlet air temperature (IAT).
This corrects for changes in air density from movement of the temperature.
*/
TESTABLE_INLINE_STATIC uint8_t correctionIATDensity(const statuses &current, const table2D &lookupTable)
{
  // Performance: only update as fast as the sensor is read
  if( BIT_CHECK(LOOP_TIMER, IAT_READ_TIMER_BIT) ) { 
    return (uint8_t)table2D_getValue(&lookupTable, toStorageTemperature(current.IAT)); //currentStatus.IAT is the actual temperature, values in IATDensityCorrectionTable.axisX are temp+offset
  }
  return current.iatCorrection;
}

// ============================= Baro pressure correction =============================

/** Correction for current barometric / ambient pressure.
 * @returns A percentage value indicating the amount the fuelling should be changed based on the barometric reading. 100 = No change. 110 = 10% increase. 90 = 10% decrease
 */
TESTABLE_INLINE_STATIC uint8_t correctionBaro(const statuses &current, const table2D &lookupTable)
{
  // No point in updating more often than the sensor is read
  if( BIT_CHECK(LOOP_TIMER, BARO_READ_TIMER_BIT) ) { 
    return (uint8_t)table2D_getValue(&lookupTable, current.baro);
  }
  return current.baroCorrection;
}

// ============================= Launch control correction =============================

/** Launch control has a setting to increase the fuel load to assist in bringing up boost.
This simple check applies the extra fuel if we're currently launching
*/
TESTABLE_INLINE_STATIC uint8_t correctionLaunch(const statuses &current, const config6 &page6)
{
  return (BIT_CHECK(current.status2, BIT_STATUS2_HLAUNCH) || BIT_CHECK(current.status2, BIT_STATUS2_SLAUNCH)) ? (BASELINE_FUEL_CORRECTION + page6.lnchFuelAdd) : NO_FUEL_CORRECTION;
}

// ============================= Deceleration Fuel Cut Off (DFCO) correction =============================

TESTABLE_INLINE_STATIC uint8_t correctionDFCOfuel(const statuses &current, const config9 &page9)
{
  uint8_t scaleValue = NO_FUEL_CORRECTION;
  if ( BIT_CHECK(current.status1, BIT_STATUS1_DFCO) )
  {
    if ( (page9.dfcoTaperEnable == 1U) && (dfcoTaper != 0U) )
    {
      //Do a check if the user reduced the duration while active to avoid overflow
      if (dfcoTaper > page9.dfcoTaperTime) { dfcoTaper = page9.dfcoTaperTime; }
      scaleValue = (uint8_t)map(dfcoTaper, 
                                page9.dfcoTaperTime, 0, 
                                NO_FUEL_CORRECTION, page9.dfcoTaperFuel);
      if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { --dfcoTaper; }
    }
    else { scaleValue = 0; } //Taper ended or disabled, disable fuel
  }
  else { dfcoTaper = page9.dfcoTaperTime; } //Keep updating the duration until DFCO is active

  return scaleValue;
}

/*
 * Returns true if deceleration fuel cutoff should be on, false if its off
 */
TESTABLE_INLINE_STATIC bool correctionDFCO(const statuses &current, const config2 &page2, const config4 &page4)
{
  bool DFCOValue = false;
  if ( page2.dfcoEnabled == 1U )
  {
    static uint8_t dfcoDelay;

    if ( BIT_CHECK(current.status1, BIT_STATUS1_DFCO) ) 
    {
      DFCOValue = ( current.RPM > toWorkingU8U16(RPM_MEDIUM, page4.dfcoRPM) ) && ( current.TPS < page4.dfcoTPSThresh ); 
      if ( DFCOValue == false) { dfcoDelay = 0; }
    }
    else 
    {
      if ( (current.TPS < page4.dfcoTPSThresh) 
        && (current.coolant >= toWorkingTemperature(page2.dfcoMinCLT)) 
        && (current.RPM > (toWorkingU8U16(RPM_MEDIUM, page4.dfcoRPM) + toWorkingU8U16(RPM_FINE, page4.dfcoHyster))) )
      {
        if( dfcoDelay < page2.dfcoDelay )
        {
          if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { ++dfcoDelay; }
        }
        else { DFCOValue = true; }
      }
      else { dfcoDelay = 0; } //Prevent future activation right away if previous time wasn't activated
    } // DFCO active check
  } // DFCO enabled check
  return DFCOValue;
}

// ============================= Flex fuel correction =============================

/** Flex fuel adjustment to vary fuel based on ethanol content.
 * The amount of extra fuel required is a linear relationship based on the % of ethanol.
*/
TESTABLE_INLINE_STATIC uint8_t correctionFlex(const statuses &current, const config2 &page2, const table2D &lookUptable)
{
  return page2.flexEnabled==1U ? (uint8_t)table2D_getValue(&lookUptable, current.ethanolPct) : NO_FUEL_CORRECTION;
}

// ============================= Fuel temperature correction =============================

/*
 * Fuel temperature adjustment to vary fuel based on fuel temperature reading
*/
TESTABLE_INLINE_STATIC uint8_t correctionFuelTemp(void)
{
  return configPage2.flexEnabled==1U ? (uint8_t)table2D_getValue(&fuelTempTable, toStorageTemperature(currentStatus.fuelTemp)) : NO_FUEL_CORRECTION;
}


// ============================= Air Fuel Ratio (AFR) correction =============================

uint8_t calculateAfrTarget(const table3d16RpmLoad &afrLookUpTable, const statuses &current, const config2 &page2, const config6 &page6) {
  //afrTarget value lookup must be done if O2 sensor is enabled, and always if incorporateAFR is enabled
  if (page2.incorporateAFR == true) {
    return get3DTableValue(&afrLookUpTable, current.fuelLoad, current.RPM);
  }
  if (page6.egoType!=EGO_TYPE_OFF) 
  {
    //Determine whether the Y axis of the AFR target table tshould be MAP (Speed-Density) or TPS (Alpha-N)
    //Note that this should only run after the sensor warmup delay when using Include AFR option,
    if( current.runSecs > page6.ego_sdelay) { 
      return get3DTableValue(&afrLookUpTable, current.fuelLoad, current.RPM); 
    }
    return current.O2; //Catch all
  }
  return current.afrTarget;
}

static inline uint8_t computeSimpleLeanCorrection(const statuses &current, const config6 &page6) {
  if(current.egoCorrection < (BASELINE_FUEL_CORRECTION + page6.egoLimit) ) //Fuelling adjustment must be at most the egoLimit amount (up or down)
  {
    return current.egoCorrection+1U; //Increase the fuelling by 1%
  }
  return current.egoCorrection; //Means we're at the maximum adjustment amount, so simply return that again
}

static inline uint8_t computeSimpleRichCorrection(const statuses &current, const config6 &page6) {
  if(current.egoCorrection > (BASELINE_FUEL_CORRECTION - page6.egoLimit) ) //Fuelling adjustment must be at most the egoLimit amount (up or down)
  {
    return (current.egoCorrection - 1U); //Decrease the fuelling by 1%
  }
  return current.egoCorrection; //Means we're at the maximum adjustment amount, so simply return that again  
}

static inline uint8_t computeSimpleCorrection(const statuses &current, const config6 &page6) {
  if(current.O2 > current.afrTarget) {
    return computeSimpleLeanCorrection(current, page6);
  }
  if(current.O2 < current.afrTarget) {
    return computeSimpleRichCorrection(current, page6);
  }
  return current.egoCorrection; //Means we're already right on target
}

static inline uint8_t computePIDCorrection(const statuses &current, const config6 &page6) {
  //Set the limits again, just in case the user has changed them since the last loop. 
  //Note that these are sent to the PID library as (Eg:) -15 and +15
  egoPID.SetOutputLimits(-page6.egoLimit, page6.egoLimit); 
  //Set the PID values again, just in case the user has changed them since the last loop
  egoPID.SetTunings(page6.egoKP, page6.egoKI, page6.egoKD); 
  PID_O2 = (long)(current.O2);
  PID_AFRTarget = (long)(current.afrTarget);

  (void)egoPID.Compute();
  // Can't do this in one step: MISRA compliance.
  int8_t correction = (int8_t)BASELINE_FUEL_CORRECTION + (int8_t)PID_output;
  return (uint8_t)correction;
}

static inline bool nextAfrCycleHasStarted(void) {
  return (ignitionCount >= AFRnextCycle)
      // IgnitionCount has rolled over between checks. 
      || (ignitionCount < (AFRnextCycle - configPage6.egoCount));
}

static inline void setNextAfrCycle(void) {
  AFRnextCycle = ignitionCount + configPage6.egoCount; //Set the target ignition event for the next calculation
}

static inline bool isAfrClosedLoopOperational(const statuses &current, const config6 &page6, const config9 &page9) {
  return (current.coolant > toWorkingTemperature(page6.egoTemp)) 
      && (current.RPM > toWorkingU8U16(RPM_COARSE, page6.egoRPM)) 
      && (current.TPS <= page6.egoTPSMax) 
      && (current.O2 < page6.ego_max) 
      && (current.O2 > page6.ego_min) 
      && (current.runSecs > page6.ego_sdelay) 
      && (BIT_CHECK(current.status1, BIT_STATUS1_DFCO)==false) 
      && (current.MAP <= (long)toWorkingU8U16(MAP, page9.egoMAPMax)) 
      && (current.MAP >= (long)toWorkingU8U16(MAP, page9.egoMAPMin))
      ;
}

static inline bool isValidEgoAlgorithm(const config6 &page6) {
  return (page6.egoAlgorithm != EGO_ALGORITHM_INVALID1)
      && (page6.egoAlgorithm != EGO_ALGORITHM_NONE);
}

static inline bool isAfrCorrectionEnabled(const statuses &current, const config6 &page6) {
  return (page6.egoType!=EGO_TYPE_OFF) 
      // If DFCO is active do not run the ego controllers to prevent iterator wind-up.
      && (BIT_CHECK(current.status1, BIT_STATUS1_DFCO) == false)
      && isValidEgoAlgorithm(page6);
}

static inline uint8_t computeAFRCorrection(const statuses &current, const config6 &page6) {
  uint8_t correction = NO_FUEL_CORRECTION;

  if (page6.egoAlgorithm == EGO_ALGORITHM_SIMPLE) {
    correction = computeSimpleCorrection(current, page6);
  } else if(page6.egoAlgorithm == EGO_ALGORITHM_PID) {
    correction = computePIDCorrection(current, page6);
  } else {
    // Unknown algorithm - use default & keep MISRA checker happy;
  }

  return correction;
}

/** Lookup the AFR target table and perform either a simple or PID adjustment based on this.

Simple (Best suited to narrowband sensors):
If the O2 sensor reports that the mixture is lean/rich compared to the desired AFR target, it will make a 1% adjustment
It then waits egoDelta number of ignition events and compares O2 against the target table again. If it is still lean/rich then the adjustment is increased to 2%.

This continues until either:
- the O2 reading flips from lean to rich, at which point the adjustment cycle starts again at 1% or
- the adjustment amount increases to egoLimit at which point it stays at this level until the O2 state (rich/lean) changes

PID (Best suited to wideband sensors):

*/
TESTABLE_INLINE_STATIC uint8_t correctionAFRClosedLoop(void)
{
  uint8_t correction = NO_FUEL_CORRECTION;
  
  if (isAfrCorrectionEnabled(currentStatus, configPage6)) {
    if (nextAfrCycleHasStarted()) {
      setNextAfrCycle();
        
      //Check all other requirements for closed loop adjustments
      if (isAfrClosedLoopOperational(currentStatus, configPage6, configPage9)) {
        correction = computeAFRCorrection(currentStatus, configPage6);
      }
    } else {
      // Not within the upcoming cycle, so reuse current correction
      correction = currentStatus.egoCorrection; 
    }
  } //egoType

  return correction;
}


static inline uint32_t combineCorrections(uint32_t sumCorrections, uint16_t correction) {
  if (correction == NO_FUEL_CORRECTION) {
    return sumCorrections;
  }
  return percentage(correction, sumCorrections);
}

/** Dispatch calculations for all fuel related corrections.
Calls all the other corrections functions and combines their results.
This is the only function that should be called from anywhere outside the file
*/
uint16_t correctionsFuel(void)
{
  //The values returned by each of the correction functions are multiplied together and then divided back to give a single 0-255 value.
  currentStatus.wueCorrection = correctionWUE(currentStatus, WUETable);
  uint32_t sumCorrections = currentStatus.wueCorrection;

  currentStatus.ASEValue = correctionASE(currentStatus, ASECountTable, ASETable, configPage2);
  sumCorrections = combineCorrections(sumCorrections, currentStatus.ASEValue);

  sumCorrections = combineCorrections(sumCorrections, correctionCranking(currentStatus, crankingEnrichTable, configPage10));

  currentStatus.AEamount = correctionAccel(currentStatus, configPage2, taeTable, maeTable);
  if ( (configPage2.aeApplyMode == AE_MODE_MULTIPLIER) || BIT_CHECK(currentStatus.engine, BIT_ENGINE_DCC) ) // multiply by the AE amount in case of multiplier AE mode or Decel
  {
    sumCorrections = combineCorrections(sumCorrections, currentStatus.AEamount);
  }

  sumCorrections = combineCorrections(sumCorrections, correctionFloodClear(currentStatus, configPage4));

  currentStatus.egoCorrection = correctionAFRClosedLoop();
  sumCorrections = combineCorrections(sumCorrections, currentStatus.egoCorrection);
  
  currentStatus.batCorrection = correctionBatVoltage(currentStatus, injectorVCorrectionTable, configPage2);
  sumCorrections = combineCorrections(sumCorrections, currentStatus.batCorrection);

  currentStatus.iatCorrection = correctionIATDensity(currentStatus, IATDensityCorrectionTable);
  sumCorrections = combineCorrections(sumCorrections, currentStatus.iatCorrection);

  currentStatus.baroCorrection = correctionBaro(currentStatus, baroFuelTable);
  sumCorrections = combineCorrections(sumCorrections, currentStatus.baroCorrection);

  currentStatus.flexCorrection = correctionFlex(currentStatus, configPage2, flexFuelTable);
  sumCorrections = combineCorrections(sumCorrections, currentStatus.flexCorrection);

  currentStatus.fuelTempCorrection = correctionFuelTemp();
  sumCorrections = combineCorrections(sumCorrections, currentStatus.fuelTempCorrection);

  currentStatus.launchCorrection = correctionLaunch(currentStatus, configPage6);
  sumCorrections = combineCorrections(sumCorrections, currentStatus.launchCorrection);

  BIT_WRITE(currentStatus.status1, BIT_STATUS1_DFCO, correctionDFCO(currentStatus, configPage2, configPage4));
  sumCorrections = combineCorrections(sumCorrections, correctionDFCOfuel(currentStatus, configPage9));

  //This is the maximum allowable increase
  return min((uint16_t)1500U, (uint16_t)sumCorrections);
}

//******************************** IGNITION ADVANCE CORRECTIONS ********************************

/** Correct ignition timing to configured fixed value.
 * Must be called near end to override all other corrections.
 */
int8_t correctionFixedTiming(int8_t advance)
{
  return (configPage2.fixAngEnable == 1U) ? configPage4.FixAng : advance; //Check whether the user has set a fixed timing angle
}

/** Ignition correction for coolant temperature (CLT).
 */
TESTABLE_INLINE_STATIC int8_t correctionCLTadvance(int8_t advance)
{
  static int8_t cachedValue = 0U;  // Setting this to non-zero will use additional RAM for static initialisation
  // Performance: only update as fast as the sensor is read
  if( BIT_CHECK(LOOP_TIMER, CLT_READ_TIMER_BIT) ) { 
    cachedValue = (int8_t)toWorkingU8S16(IGNITION_ADVANCE_SMALL,  (uint8_t)(table2D_getValue(&CLTAdvanceTable, toStorageTemperature(currentStatus.coolant))));
  }
  return advance + cachedValue;
}

/** Correct ignition timing to configured fixed value to use during craning.
 * Must be called near end to override all other corrections.
 */
int8_t correctionCrankingFixedTiming(int8_t advance)
{
  if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
  { 
    if ( configPage2.crkngAddCLTAdv == 0U ) { 
      advance = configPage4.CrankAng; //Use the fixed cranking ignition angle
    } else { 
      advance = correctionCLTadvance(configPage4.CrankAng); //Use the CLT compensated cranking ignition angle
    }
  }
  return advance;
}

TESTABLE_INLINE_STATIC int8_t correctionFlexTiming(int8_t advance)
{
  if( configPage2.flexEnabled == 1U ) //Check for flex being enabled
  {
    //This gets cast to a signed 8 bit value to allows for negative advance (ie retard) values here.
    currentStatus.flexIgnCorrection = (int8_t)toWorkingU8S16(IGNITION_ADVANCE_LARGE, table2D_getValue(&flexAdvTable, currentStatus.ethanolPct));
    advance = advance + currentStatus.flexIgnCorrection;
  }
  return advance;
}

static inline bool isWMIAdvanceEnabled(void) {
  return (configPage10.wmiEnabled == 1U) 
      && (configPage10.wmiAdvEnabled == 1U) 
      && (BIT_CHECK(currentStatus.status4, BIT_STATUS4_WMI_EMPTY)==false);
}

static inline bool isWMIAdvanceOperational(void) {
  return (currentStatus.TPS >= configPage10.wmiTPS) 
      && (currentStatus.RPM >= configPage10.wmiRPM) 
      && (currentStatus.MAP >= (int32_t)toWorkingU8S16(MAP, configPage10.wmiMAP)) 
      && (toStorageTemperature(currentStatus.IAT) >= configPage10.wmiIAT);
}

TESTABLE_INLINE_STATIC int8_t correctionWMITiming(int8_t advance)
{
    // TODO: limit rate to MAP update
  if(isWMIAdvanceEnabled() && isWMIAdvanceOperational()) {
    advance = advance + (int8_t)toWorkingU8S16(IGNITION_ADVANCE_LARGE, table2D_getValue(&wmiAdvTable, toRawS8(MAP, currentStatus.MAP)));
  }

  return advance;
}

/** 
 * Ignition correction for inlet air temperature (IAT).
 */
TESTABLE_INLINE_STATIC int8_t correctionIATretard(int8_t advance)
{
  static uint8_t cachedValue = 0U; // Setting this to non-zero will use additional RAM for static initialisation
  // Performance: only update as fast as the sensor is read
  if( BIT_CHECK(LOOP_TIMER, IAT_READ_TIMER_BIT) ) { 
    cachedValue = (uint8_t)table2D_getValue(&IATRetardTable, currentStatus.IAT);// TODO: check this if should be converted
  }
  return (int16_t)advance - (int16_t)cachedValue;
}


/** Ignition Idle advance correction.
 */
static constexpr uint16_t IGN_IDLE_THRESHOLD = 200U; //RPM threshold (below CL idle target) for when ign based idle control will engage

static inline uint8_t computeIdleAdvanceRpmDelta(void) {
  static constexpr int16_t DELTA_HYSTERISIS = (int16_t)toRawU8(RPM_MEDIUM, 500);
  int16_t idleRPMdelta = ((int16_t)currentStatus.CLIdleTarget - (int16_t)toRawU8(RPM_MEDIUM, currentStatus.RPM) ) + DELTA_HYSTERISIS;
  // Limit idle rpm delta between 0rpm - 1000rpm
  static constexpr int16_t DELTA_RPM_MAX = (int16_t)toRawU8(RPM_MEDIUM, 1000);
  return (uint8_t)constrain(idleRPMdelta, 0, DELTA_RPM_MAX);
}

static inline int8_t applyIdleAdvanceAdjust(int8_t advance, int8_t adjustment) {
  if(configPage2.idleAdvEnabled == IDLEADVANCE_MODE_ADDED) { 
    return (advance + adjustment); 
  } else if(configPage2.idleAdvEnabled == IDLEADVANCE_MODE_SWITCHED) { 
    return adjustment;
  } else {
    // Unknown idle advance mode - do nothing
    return advance;
  }
}

static inline bool isIdleAdvanceOn(void) {
  return (configPage2.idleAdvEnabled != IDLEADVANCE_MODE_OFF) 
      && (runSecsX10 >= toWorkingU8U16(TIME_TWENTY_MILLIS, configPage2.idleAdvDelay ))
      && (BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN))
      /* When Idle advance is the only idle speed control mechanism, activate as soon as not cranking. 
      When some other mechanism is also present, wait until the engine is no more than 200 RPM below idle target speed on first time
      */
      && ((configPage6.iacAlgorithm == IAC_ALGORITHM_NONE) 
        || (currentStatus.RPM > (toWorkingU8U16(RPM_MEDIUM, currentStatus.CLIdleTarget) - IGN_IDLE_THRESHOLD)));
}

static inline bool isIdleAdvanceOperational(void) {
  return (currentStatus.RPM < toWorkingU8U16(RPM_COARSE, configPage2.idleAdvRPM))
      && ((configPage2.vssMode == VSS_MODE_OFF) || (currentStatus.vss < configPage2.idleAdvVss))
      && (((configPage2.idleAdvAlgorithm == IDLEADVANCE_ALGO_TPS) && (currentStatus.TPS < configPage2.idleAdvTPS)) 
        || ((configPage2.idleAdvAlgorithm == IDLEADVANCE_ALGO_CTPS) && (currentStatus.CTPSActive == true)));// closed throttle position sensor (CTPS) based idle state
}

TESTABLE_INLINE_STATIC int8_t correctionIdleAdvance(int8_t advance)
{
  //Adjust the advance based on idle target rpm.
  if (isIdleAdvanceOn())
  {
    static uint8_t idleAdvDelayCount;
    if(isIdleAdvanceOperational())
    {
      if( idleAdvDelayCount < configPage9.idleAdvStartDelay )
      {
        if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { ++idleAdvDelayCount; }
      }
      else
      {
        int16_t advanceIdleAdjust = toWorkingU8S16(IGNITION_ADVANCE_SMALL, (uint8_t)table2D_getValue(&idleAdvanceTable, computeIdleAdvanceRpmDelta()));
        advance = applyIdleAdvanceAdjust(advance, (int8_t)advanceIdleAdjust); 
      }
    }
    else { idleAdvDelayCount = 0; }
  }

  return advance;
}

/** Ignition soft revlimit correction.
 */
static inline int8_t calculateSoftRevLimitAdvance(int8_t advance) {
  if (configPage2.SoftLimitMode == SOFT_LIMIT_RELATIVE) { 
    return advance - (int8_t)configPage4.SoftLimRetard; //delay timing by configured number of degrees in relative mode
  } else if (configPage2.SoftLimitMode == SOFT_LIMIT_FIXED) { 
    return (int8_t)configPage4.SoftLimRetard; //delay timing to configured number of degrees in fixed mode
  } else {
    // Unknown limit mode - do nothing, keep MISRA checker happy
    return advance;
  }
}

TESTABLE_INLINE_STATIC int8_t correctionSoftRevLimit(int8_t advance)
{
  BIT_CLEAR(currentStatus.status2, BIT_STATUS2_SFTLIM);

  if (configPage6.engineProtectType == PROTECT_CUT_IGN || configPage6.engineProtectType == PROTECT_CUT_BOTH) 
  {
    if (currentStatus.RPMdiv100 >= configPage4.SoftRevLim) //Softcut RPM limit
    {
      BIT_SET(currentStatus.status2, BIT_STATUS2_SFTLIM);
      if( softLimitTime < configPage4.SoftLimMax )
      {
        advance = calculateSoftRevLimitAdvance(advance);
        if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { 
          ++softLimitTime; 
        }
      }
    }
    else if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { 
      softLimitTime = 0; //Only reset time at runSecsX10 update rate
    } else {
      // Nothing to do, keep MISRA checker happy.
    }
  }

  return advance;
}

/** Ignition Nitrous oxide correction.
 */
TESTABLE_INLINE_STATIC int8_t correctionNitrous(int8_t advance)
{
  //Check if nitrous is currently active
  if(configPage10.n2o_enable != NITROUS_OFF)
  {
    //Check which stage is running (if any)
    if( (currentStatus.nitrous_status == NITROUS_STAGE1) || (currentStatus.nitrous_status == NITROUS_BOTH) )
    {
      advance -= (int8_t)configPage10.n2o_stage1_retard;
    }
    if( (currentStatus.nitrous_status == NITROUS_STAGE2) || (currentStatus.nitrous_status == NITROUS_BOTH) )
    {
      advance -= (int8_t)configPage10.n2o_stage2_retard;
    }
  }

  return advance;
}
/** Ignition soft launch correction.
 */
TESTABLE_INLINE_STATIC int8_t correctionSoftLaunch(int8_t advance)
{
  //SoftCut rev limit for 2-step launch control.
  if (configPage6.launchEnabled 
  && currentStatus.clutchTrigger 
  && (currentStatus.clutchEngagedRPM < toWorkingU8U16(RPM_COARSE, configPage6.flatSArm)) 
  && (currentStatus.RPM > toWorkingU8U16(RPM_COARSE, configPage6.lnchSoftLim))
  && (currentStatus.TPS >= configPage10.lnchCtrlTPS) )
  {
    BIT_SET(currentStatus.status2, BIT_STATUS2_SLAUNCH);
    advance = configPage6.lnchRetard;
  }
  else
  {
    BIT_CLEAR(currentStatus.status2, BIT_STATUS2_SLAUNCH);
  }

  return advance;
}
/** Ignition correction for soft flat shift.
 */
TESTABLE_INLINE_STATIC int8_t correctionSoftFlatShift(int8_t advance)
{
  if(configPage6.flatSEnable 
  && currentStatus.clutchTrigger 
  && (currentStatus.clutchEngagedRPM > toWorkingU8U16(RPM_COARSE, configPage6.flatSArm))
  && (currentStatus.RPM > (currentStatus.clutchEngagedRPM - toWorkingU8U16(RPM_COARSE, configPage6.flatSSoftWin) ) ) )
  {
    BIT_SET(currentStatus.status5, BIT_STATUS5_FLATSS);
    advance = configPage6.flatSRetard;
  }
  else { BIT_CLEAR(currentStatus.status5, BIT_STATUS5_FLATSS); }

  return advance;
}

static inline uint8_t _calculateKnockRecovery(uint8_t curKnockRetard)
{
  uint8_t tmpKnockRetard = curKnockRetard;
  //Check whether we are in knock recovery
  if((micros() - knockStartTime) > (configPage10.knock_duration * 100000UL)) //knock_duration is in seconds*10
  {
    //Calculate how many recovery steps have occured since the 
    uint32_t timeInRecovery = (micros() - knockStartTime) - (configPage10.knock_duration * 100000UL);
    uint8_t recoverySteps = timeInRecovery / (configPage10.knock_recoveryStepTime * 100000UL);
    int8_t recoveryTimingAdj = 0;
    if(recoverySteps > knockLastRecoveryStep) 
    { 
      recoveryTimingAdj = (recoverySteps - knockLastRecoveryStep) * configPage10.knock_recoveryStep;
      knockLastRecoveryStep = recoverySteps;
    }

    if(recoveryTimingAdj < currentStatus.knockRetard)
    {
      //Add the timing back in provided we haven't reached the end of the recovery period
      tmpKnockRetard = currentStatus.knockRetard - recoveryTimingAdj;
    }
    else 
    {
      //Recovery is complete. Knock adjustment is set to 0 and we reset the knock status
      tmpKnockRetard = 0;
      BIT_CLEAR(currentStatus.status5, BIT_STATUS5_KNOCK_ACTIVE);
      knockStartTime = 0;
      currentStatus.knockCount = 0;
    }
  }

  return tmpKnockRetard;
}

/** Ignition knock (retard) correction.
 */
static inline int8_t correctionKnockTiming(int8_t advance)
{
  byte tmpKnockRetard = 0;

  if( (configPage10.knock_mode == KNOCK_MODE_DIGITAL)  )
  {
    //
    if(currentStatus.knockCount >= configPage10.knock_count)
    {
      if(BIT_CHECK(currentStatus.status5, BIT_STATUS5_KNOCK_ACTIVE))
      {
        //Knock retard is currently active already.
        tmpKnockRetard = currentStatus.knockRetard;

        //Check if additional knock events occured
        if(BIT_CHECK(currentStatus.status5, BIT_STATUS5_KNOCK_PULSE))
        {
          //Check if the latest event was far enough after the initial knock event to pull further timing
          if((micros() - knockStartTime) > (configPage10.knock_stepTime * 1000UL))
          {
            //Recalculate the amount timing being pulled
            currentStatus.knockCount++;
            tmpKnockRetard = configPage10.knock_firstStep + ((currentStatus.knockCount - configPage10.knock_count) * configPage10.knock_stepSize);
            knockStartTime = micros();
            knockLastRecoveryStep = 0;
          }
        }
        tmpKnockRetard = _calculateKnockRecovery(tmpKnockRetard);
      }
      else
      {
        //Knock currently inactive but needs to be active now
        knockStartTime = micros();
        tmpKnockRetard = configPage10.knock_firstStep + ((currentStatus.knockCount - configPage10.knock_count) * configPage10.knock_stepSize); //
        BIT_SET(currentStatus.status5, BIT_STATUS5_KNOCK_ACTIVE);
        knockLastRecoveryStep = 0;
      }
    }

    BIT_CLEAR(currentStatus.status5, BIT_STATUS5_KNOCK_PULSE); //Reset the knock pulse indicator
  }
  else if( (configPage10.knock_mode == KNOCK_MODE_ANALOG)  )
  {
    if(BIT_CHECK(currentStatus.status5, BIT_STATUS5_KNOCK_ACTIVE))
    {
      //Check if additional knock events occured
      //Additional knock events are when the step time has passed and the voltage remains above the threshold
      if((micros() - knockStartTime) > (configPage10.knock_stepTime * 1000UL))
      {
        //Sufficient time has passed, check the current knock value
        uint16_t tmpKnockReading = getAnalogKnock();

        if(tmpKnockReading > configPage10.knock_threshold)
        {
          currentStatus.knockCount++;
          tmpKnockRetard = configPage10.knock_firstStep + ((currentStatus.knockCount - configPage10.knock_count) * configPage10.knock_stepSize);
          knockStartTime = micros();
          knockLastRecoveryStep = 0;
        }   
      }
      tmpKnockRetard = _calculateKnockRecovery(tmpKnockRetard);
    }
    else
    {
      //If not is not currently active, we read the analog pin every 30Hz
      if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_30HZ) ) 
      { 
        uint16_t tmpKnockReading = getAnalogKnock();

        if(tmpKnockReading > configPage10.knock_threshold)
        {
          //Knock detected
          knockStartTime = micros();
          tmpKnockRetard = configPage10.knock_firstStep; //
          BIT_SET(currentStatus.status5, BIT_STATUS5_KNOCK_ACTIVE);
          knockLastRecoveryStep = 0;
        }
      }
    }
  }
  

  tmpKnockRetard = min(tmpKnockRetard, configPage10.knock_maxRetard); //Ensure the commanded retard is not higher than the maximum allowed.
  currentStatus.knockRetard = tmpKnockRetard;
  return advance - tmpKnockRetard;
}

/** Ignition DFCO taper correction.
 */
TESTABLE_INLINE_STATIC int8_t correctionDFCOignition(int8_t advance, const statuses &current, const config9 &page9)
{
  if ( (page9.dfcoTaperEnable == 1U) && BIT_CHECK(current.status1, BIT_STATUS1_DFCO) )
  {
    if ( dfcoTaper != 0U )
    {
      advance -= map(dfcoTaper, page9.dfcoTaperTime, 0, 0, page9.dfcoTaperAdvance);
    }
    else { advance -= (int8_t)page9.dfcoTaperAdvance; } //Taper ended, use full value
  }
  else { dfcoTaper = page9.dfcoTaperTime; } //Keep updating the duration until DFCO is active
  return advance;
}

/** Ignition Dwell Correction.
 */
static inline uint8_t getPulsesPerRev(void) {
  if( ( (configPage4.sparkMode == IGN_MODE_SINGLE) || 
     ((configPage4.sparkMode == IGN_MODE_ROTARY) && (configPage10.rotaryType != ROTARY_IGN_RX8)) ) 
     //No point in running this for 1 cylinder engines
     && (configPage2.nCylinders > 1U) )  {
    return configPage2.nCylinders >> 1U;
  }
  return 1U;
}

static inline uint16_t adjustDwellClosedLoop(uint16_t dwell) {
    int16_t error = dwell - currentStatus.actualDwell;
    if(dwell > (uint16_t)INT16_MAX) { dwell = (uint16_t)INT16_MAX; } //Prevent overflow when casting to signed int
    if(error > ((int16_t)dwell / 2)) { error += error; } //Double correction amount if actual dwell is less than 50% of the requested dwell
    if(error > 0) { 
      return dwell + (uint16_t)error;
    }
    return dwell;
}

uint16_t correctionsDwell(uint16_t dwell)
{
  //Initialise the actualDwell value if this is the first time being called
  if(currentStatus.actualDwell == 0U) { 
    currentStatus.actualDwell = dwell; 
  } 

  //**************************************************************************************************************************
  //Pull battery voltage based dwell correction and apply if needed
  static uint8_t dwellCorrection = ONE_HUNDRED_PCT;
  if (BIT_CHECK(LOOP_TIMER, BAT_READ_TIMER_BIT)) { // Performance: only update as fast as the sensor is read
    dwellCorrection = (uint8_t)table2D_getValue(&dwellVCorrectionTable, currentStatus.battery10);
  }
  if (dwellCorrection != ONE_HUNDRED_PCT) { 
    dwell = div100(dwell) * dwellCorrection; 
  }

  //**************************************************************************************************************************
  //Dwell error correction is a basic closed loop to keep the dwell time consistent even when adjusting its end time for the per tooth timing.
  //This is mostly of benefit to low resolution triggers at low rpm (<1500)
  if( (configPage2.perToothIgn  == true) && (configPage4.dwellErrCorrect == 1U) ) {
    dwell = adjustDwellClosedLoop(dwell);
  }

  //**************************************************************************************************************************
  /*
  Dwell limiter - If the total required dwell time per revolution is longer than the maximum time available at the current RPM, reduce dwell. This can occur if there are multiple sparks per revolution
  This only times this can occur are:
  1. Single channel spark mode where there will be nCylinders/2 sparks per revolution
  2. Rotary ignition in wasted spark configuration (FC/FD), results in 2 pulses per rev. RX-8 is fully sequential resulting in 1 pulse, so not required
  */
  uint16_t sparkDur_uS = toWorkingU8U16(TIME_TEN_MILLIS, configPage4.sparkDur);
  uint8_t pulsesPerRevolution = getPulsesPerRev();
  uint16_t dwellPerRevolution = (dwell + sparkDur_uS) * pulsesPerRevolution;
  if(dwellPerRevolution > revolutionTime)
  {
    //Possibly need some method of reducing spark duration here as well, but this is a start
    uint16_t adjustedSparkDur = udiv_32_16(sparkDur_uS * revolutionTime, dwellPerRevolution);
    dwell = (pulsesPerRevolution==1U ? revolutionTime : udiv_32_16(revolutionTime, (uint16_t)pulsesPerRevolution)) - adjustedSparkDur;
  }

  return dwell;
}

/** Dispatch calculations for all ignition related corrections.
 * @param base_advance - Base ignition advance (deg. ?)
 * @return Advance considering all (~12) individual corrections
 */
int8_t correctionsIgn(int8_t base_advance)
{
  int8_t advance;
  advance = correctionFlexTiming(base_advance);
  advance = correctionWMITiming(advance);
  advance = correctionIATretard(advance);
  advance = correctionCLTadvance(advance);
  advance = correctionIdleAdvance(advance);
  advance = correctionSoftRevLimit(advance);
  advance = correctionNitrous(advance);
  advance = correctionSoftLaunch(advance);
  advance = correctionSoftFlatShift(advance);
  advance = correctionKnockTiming(advance);

  advance = correctionDFCOignition(advance, currentStatus, configPage9);

  //Fixed timing check must go last
  advance = correctionFixedTiming(advance);
  advance = correctionCrankingFixedTiming(advance); //This overrides the regular fixed timing, must come last

  return advance;
}