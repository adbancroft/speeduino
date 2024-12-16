/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,la
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
/** @file
 * Speeduino initialisation and main loop.
 */
#include <stdint.h> //developer.mbed.org/handbook/C-Data-Types
//************************************************
#include "globals.h"
#include "speeduino.h"
#include "scheduler.h"
#include "comms.h"
#include "comms_legacy.h"
#include "comms_secondary.h"
#include "maths.h"
#include "corrections.h"
#include "timers.h"
#include "decoders.h"
#include "idle.h"
#include "auxiliaries.h"
#include "sensors.h"
#include "storage.h"
#include "crankMaths.h"
#include "init.h"
#include "utilities.h"
#include "engineProtection.h"
#include "scheduledIO.h"
#include "secondaryTables.h"
#include "comms_CAN.h"
#include "SD_logger.h"
#include "auxiliaries.h"
#include "board_definition.h"
#include "unit_testing.h"
#include RTC_LIB_H //Defined in each boards .h file

TESTABLE_INLINE_STATIC uint16_t PW(int REQ_FUEL, byte VE, long MAP, uint16_t corrections, int injOpen);
TESTABLE_INLINE_STATIC void calculateStaging(uint16_t pwLimit, uint16_t pwPrimary);

uint16_t req_fuel_uS = 0; /**< The required fuel variable (As calculated by TunerStudio) in uS */
uint16_t inj_opentime_uS = 0;

uint16_t staged_req_fuel_mult_pri = 0;
uint16_t staged_req_fuel_mult_sec = 0;   

TESTABLE_INLINE_STATIC uint16_t calculatePWLimit(void)
{
  uint32_t tempLimit = percentage(configPage2.dutyLim, getRevolutionTime(currentStatus)); //The pulsewidth limit is determined to be the duty cycle limit (Eg 85%) by the total time it takes to perform 1 revolution
  //Handle multiple squirts per rev
  if (configPage2.strokes == FOUR_STROKE) { tempLimit = tempLimit * 2; }
  //Optimise for power of two divisions where possible
  switch(currentStatus.nSquirts)
  {
    case 1:
      //No action needed
      break;
    case 2:
      tempLimit = tempLimit / 2;
      break;
    case 4:
      tempLimit = tempLimit / 4;
      break;
    case 8:
      tempLimit = tempLimit / 8;
      break;
    default:
      //Non-PoT squirts value. Perform (slow) uint32_t division
      tempLimit = tempLimit / currentStatus.nSquirts;
      break;
  }
  if(tempLimit > UINT16_MAX) { tempLimit = UINT16_MAX; }

  return tempLimit;
}

/** Lookup the ignition advance from 3D ignition table.
 * The values used to look this up will be RPM and whatever load source the user has configured.
 * 
 * @return byte The current target advance value in degrees
 */
static inline byte getAdvance1(void)
{
  byte tempAdvance = 0;
  if (configPage2.ignAlgorithm == LOAD_SOURCE_MAP) //Check which fuelling algorithm is being used
  {
    //Speed Density
    currentStatus.ignLoad = currentStatus.MAP;
  }
  else if(configPage2.ignAlgorithm == LOAD_SOURCE_TPS)
  {
    //Alpha-N
    currentStatus.ignLoad = currentStatus.TPS * 2;

  }
  else if (configPage2.fuelAlgorithm == LOAD_SOURCE_IMAPEMAP)
  {
    //IMAP / EMAP
    currentStatus.ignLoad = ((int16_t)currentStatus.MAP * 100U) / currentStatus.EMAP;
  }
  tempAdvance = get3DTableValue(&ignitionTable, currentStatus.ignLoad, currentStatus.RPM) - OFFSET_IGNITION; //As above, but for ignition advance
  tempAdvance = correctionsIgn(tempAdvance);

  return tempAdvance;
}

/** Lookup the current VE value from the primary 3D fuel map.
 * The Y axis value used for this lookup varies based on the fuel algorithm selected (speed density, alpha-n etc).
 * 
 * @return byte The current VE value
 */
static inline byte getVE1(void)
{
  byte tempVE = 100;
  if (configPage2.fuelAlgorithm == LOAD_SOURCE_MAP) //Check which fuelling algorithm is being used
  {
    //Speed Density
    currentStatus.fuelLoad = currentStatus.MAP;
  }
  else if (configPage2.fuelAlgorithm == LOAD_SOURCE_TPS)
  {
    //Alpha-N
    currentStatus.fuelLoad = currentStatus.TPS * 2;
  }
  else if (configPage2.fuelAlgorithm == LOAD_SOURCE_IMAPEMAP)
  {
    //IMAP / EMAP
    currentStatus.fuelLoad = ((int16_t)currentStatus.MAP * 100U) / currentStatus.EMAP;
  }
  else { currentStatus.fuelLoad = currentStatus.MAP; } //Fallback position
  tempVE = get3DTableValue(&fuelTable, currentStatus.fuelLoad, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value

  return tempVE;
}

/**
 * @brief This function calculates the required pulsewidth time (in us) given the current system state
 * 
 * @param REQ_FUEL The required fuel value in uS, as calculated by TunerStudio
 * @param VE Lookup from the main fuel table. This can either have been MAP or TPS based, depending on the algorithm used
 * @param MAP In KPa, read from the sensor (This is used when performing a multiply of the map only. It is applicable in both Speed density and Alpha-N)
 * @param corrections Sum of Enrichment factors (Cold start, acceleration). This is a multiplication factor (Eg to add 10%, this should be 110)
 * @param injOpen Injector opening time. The time the injector take to open minus the time it takes to close (Both in uS)
 * @return uint16_t The injector pulse width in uS
 */
TESTABLE_INLINE_STATIC uint16_t PW(int REQ_FUEL, byte VE, long MAP, uint16_t corrections, int injOpen)
{
  //Standard float version of the calculation
  //return (REQ_FUEL * (float)(VE/100.0) * (float)(MAP/100.0) * (float)(TPS/100.0) * (float)(corrections/100.0) + injOpen);
  //Note: The MAP and TPS portions are currently disabled, we use VE and corrections only
  uint16_t iVE;
  uint16_t iMAP = 100;
  uint16_t iAFR = 147;

  //100% float free version, does sacrifice a little bit of accuracy, but not much.
 
  //iVE = ((unsigned int)VE << 7) / 100;
  iVE = div100(((uint16_t)VE << 7U));

  //Check whether either of the multiply MAP modes is turned on
  //if ( configPage2.multiplyMAP == MULTIPLY_MAP_MODE_100) { iMAP = ((unsigned int)MAP << 7) / 100; }
  if ( configPage2.multiplyMAP == MULTIPLY_MAP_MODE_100) { iMAP = div100( ((uint16_t)MAP << 7U) ); }
  else if( configPage2.multiplyMAP == MULTIPLY_MAP_MODE_BARO) { iMAP = ((unsigned int)MAP << 7U) / currentStatus.baro; }
  
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == EGO_TYPE_WIDE) && (currentStatus.runSecs > configPage6.ego_sdelay) ) {
    iAFR = ((unsigned int)currentStatus.O2 << 7U) / currentStatus.afrTarget;  //Include AFR (vs target) if enabled
  }
  if ( (configPage2.incorporateAFR == true) && (configPage2.includeAFR == false) ) {
    iAFR = ((unsigned int)configPage2.stoich << 7U) / currentStatus.afrTarget;  //Incorporate stoich vs target AFR, if enabled.
  }

  uint32_t intermediate = rshift<7U>((uint32_t)REQ_FUEL * (uint32_t)iVE); //Need to use an intermediate value to avoid overflowing the long
  if ( configPage2.multiplyMAP > 0 ) { intermediate = rshift<7U>(intermediate * (uint32_t)iMAP); }
  
  if ( (configPage2.includeAFR == true) && (configPage6.egoType == EGO_TYPE_WIDE) && (currentStatus.runSecs > configPage6.ego_sdelay) ) {
    //EGO type must be set to wideband and the AFR warmup time must've elapsed for this to be used
    intermediate = rshift<7U>(intermediate * (uint32_t)iAFR);  
  }
  if ( (configPage2.incorporateAFR == true) && (configPage2.includeAFR == false) ) {
    intermediate = rshift<7U>(intermediate * (uint32_t)iAFR);
  }

  //If corrections are huge, use less bitshift to avoid overflow. Sacrifices a bit more accuracy (basically only during very cold temp cranking)
  if (corrections < 512 ) { 
    intermediate = rshift<7U>(intermediate * div100(lshift<7U>(corrections))); 
  } else if (corrections < 1024 ) { 
    intermediate = rshift<6U>(intermediate * div100(lshift<6U>(corrections)));
  } else {
    intermediate = rshift<5U>(intermediate * div100(lshift<5U>(corrections)));
  }

  if (intermediate != 0)
  {
    //If intermediate is not 0, we need to add the opening time (0 typically indicates that one of the full fuel cuts is active)
    intermediate += injOpen; //Add the injector opening time
    //AE calculation only when ACC is active.
    if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_ACC) )
    {
      //AE Adds % of req_fuel
      if ( configPage2.aeApplyMode == AE_MODE_ADDER )
        {
          intermediate += div100(((uint32_t)REQ_FUEL) * (currentStatus.AEamount - 100U));
        }
    }

    if ( intermediate > UINT16_MAX)
    {
      intermediate = UINT16_MAX;  //Make sure this won't overflow when we convert to uInt. This means the maximum pulsewidth possible is 65.535mS
    }
  }
  return (unsigned int)(intermediate);
}

#ifndef UNIT_TEST // Scope guard for unit testing

static byte ignitionChannelsOn; /**< The current state of the ignition system (on or off) */
static byte ignitionChannelsPending; /**< Any ignition channels that are pending injections before they are resumed */
static byte fuelChannelsOn; /**< The current state of the fuel system (on or off) */
static uint32_t rollingCutLastRev; /**< Tracks whether we're on the same or a different rev for the rolling cut */

// Forward declarations
static inline void calculateIgnitionAngles(int16_t dwellTime, int8_t advance, int16_t ignLoad);

void setup(void)
{
  currentStatus.initialisationComplete = false; //Tracks whether the initialiseAll() function has run completely
  initialiseAll();
}

static inline void applyFuelTrimToPW(FuelSchedule &schedule, int16_t fuelLoad, int16_t RPM)
{
  if (schedule.pw!=0U) {
    int16_t offset = (int16_t)get3DTableValue(&schedule.trimTable, fuelLoad, RPM) - (int16_t)OFFSET_FUELTRIM;
    uint8_t pw1percent = (uint8_t)(100U + (uint8_t)offset);
    if (pw1percent != 100U) { schedule.pw = percentage(pw1percent, schedule.pw); }
  }
}

#if defined(USE_IGN_REFRESH)
static inline uint16_t refreshIgnition1(uint16_t crankAngle) {
  if( isRunning(ignitionSchedules[0]) && (ignitionSchedules[0].dischargeAngle > (int16_t)crankAngle) && (configPage4.StgCycles == 0U) && (configPage2.perToothIgn != true) )
  {
    crankAngle = ignitionLimits(getCrankAngle()); //Refresh the crank angle info

    adjustCrankAngle(ignitionSchedules[0], crankAngle);
  }
  
  return crankAngle;
}
#endif

static void setIgnitionSchedules(uint16_t crankAngle, uint16_t totalDwell) {
#if defined(USE_IGN_REFRESH)
  setIgnitionSchedule(ignitionSchedules[0], crankAngle, totalDwell);
  crankAngle = refreshIgnition1(crankAngle);
  uint8_t index = 1;
#else
  uint8_t index = 0;
#endif
  for (; index < maxIgnOutputs; ++index) {
    if (BIT_CHECK(ignitionChannelsOn, (IGN1_CMD_BIT+index))) {
      setIgnitionSchedule(ignitionSchedules[index], crankAngle, totalDwell);
    }
  }
} 

static inline void matchSyncState(const config2 &page2, const statuses &current) {
  if ((page2.injLayout == INJ_SEQUENTIAL) && ((page2.nCylinders==4) || (page2.nCylinders==6) || (page2.nCylinders==8)))
  {
    if ((current.hasSync) && ( CRANK_ANGLE_MAX_INJ != 720 )) {
      changeHalfToFullSync();
    } else if( BIT_CHECK(current.status3, BIT_STATUS3_HALFSYNC) && (CRANK_ANGLE_MAX_INJ != 360) ) { 
      changeFullToHalfSync();
    } else {
      // Injection layout matches current sync - nothing to do but keep MISRA checker happy
    }
  }
}

static inline void applyFuelTrims(const config2 &page2, const config6 &page6, const statuses &current) {
  if ( (page2.injLayout == INJ_SEQUENTIAL) && (page6.fuelTrimEnabled > 0U) )
  {
    uint8_t trimInjChannels = min(page2.nCylinders, maxInjPrimaryOutputs);
    for (uint8_t index=0; index<trimInjChannels; ++index) {
      applyFuelTrimToPW(fuelSchedules[index], current.fuelLoad, current.RPM);
    }
  }
}

static void setFuelSchedules(uint16_t injAngle, uint16_t crankAngle) {
  injectorAngleCalcCache calcCache;
  for (uint8_t index=0; index<maxInjPrimaryOutputs+maxInjSecondaryOutputs; ++index) {
    if(BIT_CHECK(fuelChannelsOn, (INJ1_CMD_BIT+index))) {
      setFuelSchedule(fuelSchedules[index], injAngle, crankAngle, &calcCache);
    }
  }
}

// FixedCrankingOverride is used to extend the dwell during cranking so that the decoder can trigger the spark upon seeing a certain tooth. Currently only available on the basic distributor and 4g63 decoders.
inline uint16_t __attribute__((always_inline)) computeFixedCrankingOverride(void) {
  if ( isFixedCrankLock() )
  {
    //This is a safety step to prevent the ignition start time occurring AFTER the target tooth pulse has already occurred. It simply moves the start time forward a little, which is compensated for by the increase in the dwell time
    if(currentStatus.RPM < 250U)
    {
      for (uint8_t index=0U; index<maxIgnOutputs; ++index) {
        ignitionSchedules[index].chargeAngle -= 5;
      }
    }
    return currentStatus.dwell * 3;
  }
  return 0U;
}

/** Speeduino main loop.
 * 
 * Main loop chores (roughly in the order that they are performed):
 * - Check if serial comms or tooth logging are in progress (send or receive, prioritise communication)
 * - Record loop timing vars
 * - Check tooth time, update @ref statuses (currentStatus) variables
 * - Read sensors
 * - get VE for fuel calcs and spark advance for ignition
 * - Check crank/cam/tooth/timing sync (skip remaining ops if out-of-sync)
 * - execute doCrankSpeedCalcs()
 * 
 * single byte variable @ref LOOP_TIMER plays a big part here as:
 * - it contains expire-bits for interval based frequency driven events (e.g. 15Hz, 4Hz, 1Hz)
 * - Can be tested for certain frequency interval being expired by (eg) BIT_CHECK(LOOP_TIMER, BIT_TIMER_15HZ)
 * 
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
// Sometimes loop() is inlined by LTO & sometimes not
// When not inlined, there is a huge difference in stack usage: 60+ bytes
// That eats into available RAM.
// Adding __attribute__((always_inline, hot)) forces the LTO process to inline.
//
// Since the function is declared in an Arduino header, we can't change
// it to inline, so we need to suppress the resulting warning.
void __attribute__((always_inline, hot)) loop(void)
{
      if(mainLoopCount < UINT16_MAX) { mainLoopCount++; }
      LOOP_TIMER = TIMER_mask;

      //SERIAL Comms
      //Initially check that the last serial send values request is not still outstanding
      if (serialTransmitInProgress())
      {
        serialTransmit();
      }

      //Check for any new or in-progress requests from serial.
      if (Serial.available()>0 || serialRecieveInProgress())
      {
        serialReceive();
      }
      
      //Check for any CAN comms requiring action 
      #if defined(secondarySerial_AVAILABLE)
        //if can or secondary serial interface is enabled then check for requests.
        if (configPage9.enable_secondarySerial == 1U)  //secondary serial interface enabled
        {
          if ( ((mainLoopCount & 31U) == 1U) || (secondarySerial.available() > SERIAL_BUFFER_THRESHOLD) )
          {
            if (secondarySerial.available() > 0)  { secondserial_Command(); }
          } 
        }
      #endif
      #if defined (NATIVE_CAN_AVAILABLE)
        if (configPage9.enable_intcan == 1) // use internal can module
        {            
          //check local can module
          // if ( BIT_CHECK(LOOP_TIMER, BIT_TIMER_15HZ) or (CANbus0.available())
          while (CAN_read()) 
          {
            can_Command();
            readAuxCanBus();
            if (configPage2.canWBO > 0) { receiveCANwbo(); }
          }
        }   
      #endif
          
    if(currentLoopTime > micros_safe())
    {
      //Occurs when micros() has overflowed
      deferEEPROMWritesUntil = 0; //Required to ensure that EEPROM writes are not deferred indefinitely
    }

    currentLoopTime = micros_safe();
    if ( engineIsRunning(currentLoopTime) )
    {
      currentStatus.longRPM = getRPM(); //Long RPM is included here
      currentStatus.RPM = currentStatus.longRPM;
      currentStatus.RPMdiv100 = div100(currentStatus.RPM);
      if(currentStatus.RPM > 0)
      {
        FUEL_PUMP_ON();
        currentStatus.fuelPumpOn = true;
      }
    }
    else
    {
      //We reach here if the time between teeth is too great. This VERY likely means the engine has stopped
      currentStatus.RPM = 0;
      currentStatus.RPMdiv100 = 0;
      fuelSchedules[0].pw = 0;
      currentStatus.VE = 0;
      currentStatus.VE2 = 0;
      resetDecoder();
      currentStatus.hasSync = false;
      BIT_CLEAR(currentStatus.status3, BIT_STATUS3_HALFSYNC);
      currentStatus.runSecs = 0; //Reset the counter for number of seconds running.
      currentStatus.startRevolutions = 0;
      initialiseMAPBaro();
      currentStatus.rpmDOT = 0;
      AFRnextCycle = 0;
      ignitionCount = 0;
      ignitionChannelsOn = 0;
      fuelChannelsOn = 0;
      if (currentStatus.fpPrimed == true) { FUEL_PUMP_OFF(); currentStatus.fuelPumpOn = false; } //Turn off the fuel pump, but only if the priming is complete
      if (configPage6.iacPWMrun == false) { disableIdle(); } //Turn off the idle PWM
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK); //Clear cranking bit (Can otherwise get stuck 'on' even with 0 rpm)
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_WARMUP); //Same as above except for WUE
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_RUN); //Same as above except for RUNNING status
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ASE); //Same as above except for ASE status
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ACC); //Same as above but the accel enrich (If using MAP accel enrich a stall will cause this to trigger)
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_DCC); //Same as above but the decel enleanment
      //This is a safety check. If for some reason the interrupts have got screwed up (Leading to 0rpm), this resets them.
      //It can possibly be run much less frequently.
      //This should only be run if the high speed logger are off because it will change the trigger interrupts back to defaults rather than the logger versions
      if( (currentStatus.toothLogEnabled == false) && (currentStatus.compositeTriggerUsed == 0U) ) { initialiseTriggers(); }

      VVT1_PIN_LOW();
      VVT2_PIN_LOW();
      DISABLE_VVT_TIMER();
      boostDisable();
      if(configPage4.ignBypassEnabled > 0U) { digitalWrite(pinIgnBypass, LOW); } //Reset the ignition bypass ready for next crank attempt
    }
    //***Perform sensor reads***
    //-----------------------------------------------------------------------------------------------------
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_1KHZ)) //Every 1ms. NOTE: This is NOT guaranteed to run at 1kHz on AVR systems. It will run at 1kHz if possible or as fast as loops/s allows if not. 
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_1KHZ);
      readMAP();
    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_200HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_200HZ);
      #if defined(ANALOG_ISR)
        //ADC in free running mode does 1 complete conversion of all 16 channels and then the interrupt is disabled. Every 200Hz we re-enable the interrupt to get another conversion cycle
        BIT_SET(ADCSRA,ADIE); //Enable ADC interrupt
      #endif
    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_50HZ)) //50 hertz
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_50HZ);

      #if defined(NATIVE_CAN_AVAILABLE)
      sendCANBroadcast(50);
      #endif

    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_30HZ)) //30 hertz
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_30HZ);
      //Most boost tends to run at about 30Hz, so placing it here ensures a new target time is fetched frequently enough
      boostControl();
      //VVT may eventually need to be synced with the cam readings (ie run once per cam rev) but for now run at 30Hz
      vvtControl();
      //Water methanol injection
      wmiControl();
      #if TPS_READ_FREQUENCY == 30
        readTPS();
      #endif
      if (configPage2.canWBO == 0U)
      {
        readO2();
        readO2_2();
      }
      
      #if defined(NATIVE_CAN_AVAILABLE)
      sendCANBroadcast(30);
      #endif

      #ifdef SD_LOGGING
        if(configPage13.onboard_log_file_rate == LOGGER_RATE_30HZ) { writeSDLogEntry(); }
      #endif

      //Check for any outstanding EEPROM writes.
      if( (isEepromWritePending() == true) && (serialStatusFlag == SERIAL_INACTIVE) && (micros() > deferEEPROMWritesUntil)) { writeAllConfig(); } 
    }
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_15HZ)) //Every 32 loops
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_15HZ);
      #if TPS_READ_FREQUENCY == 15
        readTPS(); //TPS reading to be performed every 32 loops (any faster and it can upset the TPSdot sampling time)
      #endif
      #if  defined(CORE_TEENSY35)       
          if (configPage9.enable_intcan == 1) // use internal can module
          {
           // this is just to test the interface is sending
           //sendCancommand(3,((configPage9.realtime_base_address & 0x3FF)+ 0x100),currentStatus.TPS,0,0x200);
          }
      #endif     

      checkLaunchAndFlatShift(); //Check for launch control and flat shift being active

      #if defined(NATIVE_CAN_AVAILABLE)
      sendCANBroadcast(15);
      #endif

      //And check whether the tooth log buffer is ready
      if(toothHistoryIndex > TOOTH_LOG_SIZE) { BIT_SET(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY); }
    }
    if(BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ)) //10 hertz
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_10HZ);
      //updateFullStatus();
      checkProgrammableIO();
      idleControl(); //Perform any idle related actions. This needs to be run at 10Hz to align with the idle taper resolution of 0.1s
      
      // Air conditioning control
      airConControl();

      currentStatus.vss = getSpeed();
      currentStatus.gear = getGear();

      #if defined(NATIVE_CAN_AVAILABLE)
      sendCANBroadcast(10);
      #endif

      #ifdef SD_LOGGING
        if(configPage13.onboard_log_file_rate == LOGGER_RATE_10HZ) { writeSDLogEntry(); }
      #endif
    }
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_4HZ))
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_4HZ);
      //The IAT and CLT readings can be done less frequently (4 times per second)
      readCLT();
      readIAT();
      readBat();
      nitrousControl();

      //Lookup the current target idle RPM. This is aligned with coolant and so needs to be calculated at the same rate CLT is read
      if( (configPage2.idleAdvEnabled >= 1U) || (configPage6.iacAlgorithm != IAC_ALGORITHM_NONE) )
      {
        currentStatus.CLIdleTarget = (byte)table2D_getValue(&idleTargetTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
        if(BIT_CHECK(currentStatus.airConStatus, BIT_AIRCON_TURNING_ON)) { currentStatus.CLIdleTarget += configPage15.airConIdleUpRPMAdder;  } //Adds Idle Up RPM amount if active
      }

      #ifdef SD_LOGGING
        if(configPage13.onboard_log_file_rate == LOGGER_RATE_4HZ) { writeSDLogEntry(); }
        syncSDLog(); //Sync the SD log file to the card 4 times per second. 
      #endif  
      
      currentStatus.fuelPressure = getFuelPressure();
      currentStatus.oilPressure = getOilPressure();
      
      if(auxIsEnabled == true)
      {
        //TODO dazq to clean this right up :)
        //check through the Aux input channels if enabled for Can or local use
        for (byte AuxinChan = 0U; AuxinChan <16U ; AuxinChan++)
        {
          currentStatus.current_caninchannel = AuxinChan;          
          
          if (((configPage9.caninput_sel[currentStatus.current_caninchannel]&12) == 4) 
              && (((configPage9.enable_secondarySerial == 1) && ((configPage9.enable_intcan == 0)&&(configPage9.intcan_available == 1)))
              || ((configPage9.enable_secondarySerial == 1) && ((configPage9.enable_intcan == 1)&&(configPage9.intcan_available == 1))&& 
              ((configPage9.caninput_sel[currentStatus.current_caninchannel]&64) == 0))
              || ((configPage9.enable_secondarySerial == 1) && ((configPage9.enable_intcan == 1)&&(configPage9.intcan_available == 0)))))              
          { //if current input channel is enabled as external & secondary serial enabled & internal can disabled(but internal can is available)
            // or current input channel is enabled as external & secondary serial enabled & internal can enabled(and internal can is available)
            //currentStatus.canin[13] = 11;  Dev test use only!
            if (configPage9.enable_secondarySerial == 1)  // megas only support can via secondary serial
            {
              sendCancommand(2,0,currentStatus.current_caninchannel,0,((configPage9.caninput_source_can_address[currentStatus.current_caninchannel]&2047)+0x100));
              //send an R command for data from caninput_source_address[currentStatus.current_caninchannel] from secondarySerial
            }
          }  
          else if (((configPage9.caninput_sel[currentStatus.current_caninchannel]&12) == 4) 
              && (((configPage9.enable_secondarySerial == 1) && ((configPage9.enable_intcan == 1)&&(configPage9.intcan_available == 1))&& 
              ((configPage9.caninput_sel[currentStatus.current_caninchannel]&64) == 64))
              || ((configPage9.enable_secondarySerial == 0) && ((configPage9.enable_intcan == 1)&&(configPage9.intcan_available == 1))&& 
              ((configPage9.caninput_sel[currentStatus.current_caninchannel]&128) == 128))))                             
          { //if current input channel is enabled as external for canbus & secondary serial enabled & internal can enabled(and internal can is available)
            // or current input channel is enabled as external for canbus & secondary serial disabled & internal can enabled(and internal can is available)
            //currentStatus.canin[13] = 12;  Dev test use only!  
          #if defined(CORE_STM32) || defined(CORE_TEENSY)
           if (configPage9.enable_intcan == 1) //  if internal can is enabled 
           {
              sendCancommand(3,configPage9.speeduino_tsCanId,currentStatus.current_caninchannel,0,((configPage9.caninput_source_can_address[currentStatus.current_caninchannel]&2047)+0x100));  
              //send an R command for data from caninput_source_address[currentStatus.current_caninchannel] from internal canbus
           }
          #endif
          }   
          else if ((((configPage9.enable_secondarySerial == 1) || ((configPage9.enable_intcan == 1) && (configPage9.intcan_available == 1))) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&12) == 8)
                  || (((configPage9.enable_secondarySerial == 0) && ( (configPage9.enable_intcan == 1) && (configPage9.intcan_available == 0) )) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 2)  
                  || (((configPage9.enable_secondarySerial == 0) && (configPage9.enable_intcan == 0)) && ((configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 2)))  
          { //if current input channel is enabled as analog local pin
            //read analog channel specified
            //currentStatus.canin[13] = (configPage9.Auxinpina[currentStatus.current_caninchannel]&63);  Dev test use only!127
            currentStatus.canin[currentStatus.current_caninchannel] = readAuxanalog(pinTranslateAnalog(configPage9.Auxinpina[currentStatus.current_caninchannel]&63));
          }
          else if ((((configPage9.enable_secondarySerial == 1) || ((configPage9.enable_intcan == 1) && (configPage9.intcan_available == 1))) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&12) == 12)
                  || (((configPage9.enable_secondarySerial == 0) && ( (configPage9.enable_intcan == 1) && (configPage9.intcan_available == 0) )) && (configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 3)
                  || (((configPage9.enable_secondarySerial == 0) && (configPage9.enable_intcan == 0)) && ((configPage9.caninput_sel[currentStatus.current_caninchannel]&3) == 3)))
          { //if current input channel is enabled as digital local pin
            //read digital channel specified
            //currentStatus.canin[14] = ((configPage9.Auxinpinb[currentStatus.current_caninchannel]&63)+1);  Dev test use only!127+1
            currentStatus.canin[currentStatus.current_caninchannel] = readAuxdigital((configPage9.Auxinpinb[currentStatus.current_caninchannel]&63)+1);
          } //Channel type
          else {
            // Unknown channel type - do nothing but keep MISRA happy
          }
        } //For loop going through each channel
      } //aux channels are enabled
    } //4Hz timer
    if (BIT_CHECK(LOOP_TIMER, BIT_TIMER_1HZ)) //Once per second)
    {
      BIT_CLEAR(TIMER_mask, BIT_TIMER_1HZ);
      readBaro(); //Infrequent baro readings are not an issue.

      if ( (configPage10.wmiEnabled > 0U) && (configPage10.wmiIndicatorEnabled > 0U) )
      {
        // water tank empty
        if (BIT_CHECK(currentStatus.status4, BIT_STATUS4_WMI_EMPTY) != 0U)
        {
          // flash with 1sec interval
          digitalWrite(pinWMIIndicator, !digitalRead(pinWMIIndicator));
        }
        else
        {
          digitalWrite(pinWMIIndicator, configPage10.wmiIndicatorPolarity ? HIGH : LOW);
        } 
      }

      #ifdef SD_LOGGING
        if(configPage13.onboard_log_file_rate == LOGGER_RATE_1HZ) { writeSDLogEntry(); }
      #endif

    } //1Hz timer

    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OL)
    || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_CL)
    || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OLCL) )
    {
      idleControl(); //Run idlecontrol every loop for stepper idle.
    }

    
    //VE and advance calculation were moved outside the sync/RPM check so that the fuel and ignition load value will be accurately shown when RPM=0
    currentStatus.VE1 = getVE1();
    currentStatus.VE = currentStatus.VE1; //Set the final VE value to be VE 1 as a default. This may be changed in the section below

    currentStatus.advance1 = getAdvance1();
    currentStatus.advance = currentStatus.advance1; //Set the final advance value to be advance 1 as a default. This may be changed in the section below

    calculateSecondaryFuel();
    calculateSecondarySpark();

    //Always check for sync
    //Main loop runs within this clause
    if ((currentStatus.hasSync || BIT_CHECK(currentStatus.status3, BIT_STATUS3_HALFSYNC)) && (currentStatus.RPM > 0U))
    {
        //Check whether running or cranking
        if(currentStatus.RPM > currentStatus.crankRPM) //Crank RPM in the config is stored as a x10. currentStatus.crankRPM is set in timers.ino and represents the true value
        {
          BIT_SET(currentStatus.engine, BIT_ENGINE_RUN); //Sets the engine running bit
          //Only need to do anything if we're transitioning from cranking to running
          if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
          {
            BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
            if(configPage4.ignBypassEnabled > 0U) { digitalWrite(pinIgnBypass, HIGH); }
          }
        }
        else
        {  
          if( !BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN) || (currentStatus.RPM < (currentStatus.crankRPM - CRANK_RUN_HYSTER)) )
          {
            //Sets the engine cranking bit, clears the engine running bit
            BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK);
            BIT_CLEAR(currentStatus.engine, BIT_ENGINE_RUN);
            currentStatus.runSecs = 0; //We're cranking (hopefully), so reset the engine run time to prompt ASE.
            if(configPage4.ignBypassEnabled > 0U) { digitalWrite(pinIgnBypass, LOW); }

            //Check whether the user has selected to disable to the fan during cranking
            if(configPage2.fanWhenCranking == 0U) { FAN_OFF(); }
          }
        }
      //END SETTING ENGINE STATUSES
      //-----------------------------------------------------------------------------------------------------

      //Begin the fuel calculation
      //Calculate an injector pulsewidth from the VE
      currentStatus.afrTarget = calculateAfrTarget(afrTable, currentStatus, configPage2, configPage6);
      currentStatus.corrections = correctionsFuel();

      uint16_t primaryPW = PW(req_fuel_uS, currentStatus.VE, currentStatus.MAP, currentStatus.corrections, inj_opentime_uS);

      //Manual adder for nitrous. These are not in correctionsFuel() because they are direct adders to the ms value, not % based
      if( (currentStatus.nitrous_status == NITROUS_STAGE1) || (currentStatus.nitrous_status == NITROUS_BOTH) )
      { 
        int16_t adderRange = (configPage10.n2o_stage1_maxRPM - configPage10.n2o_stage1_minRPM) * 100;
        int16_t adderPercent = ((currentStatus.RPM - (configPage10.n2o_stage1_minRPM * 100)) * 100) / adderRange; //The percentage of the way through the RPM range
        adderPercent = 100 - adderPercent; //Flip the percentage as we go from a higher adder to a lower adder as the RPMs rise
        primaryPW = primaryPW + (configPage10.n2o_stage1_adderMax + percentage(adderPercent, (configPage10.n2o_stage1_adderMin - configPage10.n2o_stage1_adderMax))) * 100; //Calculate the above percentage of the calculated ms value.
      }
      if( (currentStatus.nitrous_status == NITROUS_STAGE2) || (currentStatus.nitrous_status == NITROUS_BOTH) )
      {
        int16_t adderRange = (configPage10.n2o_stage2_maxRPM - configPage10.n2o_stage2_minRPM) * 100;
        int16_t adderPercent = ((currentStatus.RPM - (configPage10.n2o_stage2_minRPM * 100)) * 100) / adderRange; //The percentage of the way through the RPM range
        adderPercent = 100 - adderPercent; //Flip the percentage as we go from a higher adder to a lower adder as the RPMs rise
        primaryPW = primaryPW + (configPage10.n2o_stage2_adderMax + percentage(adderPercent, (configPage10.n2o_stage2_adderMin - configPage10.n2o_stage2_adderMax))) * 100; //Calculate the above percentage of the calculated ms value.
      }
      
      calculateStaging(calculatePWLimit(), primaryPW);

      //***********************************************************************************************
      //BEGIN INJECTION TIMING
      currentStatus.injAngle = injectorLimits(table2D_getValue(&injectorAngleTable, currentStatus.RPMdiv100));

      matchSyncState(configPage2, currentStatus);
      applyFuelTrims(configPage2, configPage6, currentStatus);

      //***********************************************************************************************
      //| BEGIN IGNITION CALCULATIONS

      //Set dwell
      //Dwell is stored as ms * 10. ie Dwell of 4.3ms would be 43 in configPage4. This number therefore needs to be multiplied by 100 to get dwell in uS
      if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) ) {
        currentStatus.dwell =  (configPage4.dwellCrank * 100U); //use cranking dwell
      }
      else 
      {
        if ( configPage2.useDwellMap == true )
        {
          currentStatus.dwell = (get3DTableValue(&dwellTable, currentStatus.ignLoad, currentStatus.RPM) * 100U); //use running dwell from map
        }
        else
        {
          currentStatus.dwell =  (configPage4.dwellRun * 100U); //use fixed running dwell
        }
      }
      currentStatus.dwell = correctionsDwell(currentStatus.dwell);

      calculateIgnitionAngles(currentStatus.dwell, currentStatus.advance, currentStatus.ignLoad);

      //***********************************************************************************************
      //| BEGIN FUEL SCHEDULES
      //Finally calculate the time (uS) until we reach the firing angles and set the schedules
      //We only need to set the schedule if we're BEFORE the open angle
      //This may potentially be called a number of times as we get closer and closer to the opening time

      // if(Serial && false)
      // {
      //   if(ignitionSchedules[0].chargeAngle > crankAngle)
      //   {
      //     noInterrupts();
      //     Serial.print("Time2LastTooth:"); Serial.println(micros()-toothLastToothTime);
      //     Serial.print("elapsedTime:"); Serial.println(elapsedTime);
      //     Serial.print("CurAngle:"); Serial.println(crankAngle);
      //     Serial.print("RPM:"); Serial.println(currentStatus.RPM);
      //     Serial.print("Tooth:"); Serial.println(toothCurrentCount);
      //     Serial.print("timePerDegree:"); Serial.println(timePerDegree);
      //     Serial.print("IGN1Angle:"); Serial.println(ignitionSchedules[0].chargeAngle);
      //     Serial.print("TimeToIGN1:"); Serial.println(angleToTime((ignitionSchedules[0].chargeAngle - crankAngle), CRANKMATH_METHOD_INTERVAL_REV));
      //     interrupts();
      //   }
      // }
      
      //Check for any of the engine protections or rev limiters being turned on
      uint16_t maxAllowedRPM = checkRevLimit(); //The maximum RPM allowed by all the potential limiters (Engine protection, 2-step, flat shift etc). Divided by 100. `checkRevLimit()` returns the current maximum RPM allow (divided by 100) based on either the fixed hard limit or the current coolant temp
      //Check each of the functions that has an RPM limit. Update the max allowed RPM if the function is active and has a lower RPM than already set
      if( checkEngineProtect() && (configPage4.engineProtectMaxRPM < maxAllowedRPM)) { maxAllowedRPM = configPage4.engineProtectMaxRPM; }
      if ( (currentStatus.launchingHard == true) && (configPage6.lnchHardLim < maxAllowedRPM) ) { maxAllowedRPM = configPage6.lnchHardLim; }
      maxAllowedRPM = maxAllowedRPM * 100U; //All of the above limits are divided by 100, convert back to RPM
      if ( (currentStatus.flatShiftingHard == true) && (currentStatus.clutchEngagedRPM < maxAllowedRPM) ) { maxAllowedRPM = currentStatus.clutchEngagedRPM; } //Flat shifting is a special case as the RPM limit is based on when the clutch was engaged. It is not divided by 100 as it is set with the actual RPM
    
      if( (configPage2.hardCutType == HARD_CUT_FULL) && (currentStatus.RPM > maxAllowedRPM) )
      {
        //Full hard cut turns outputs off completely. 
        switch(configPage6.engineProtectType)
        {
          case PROTECT_CUT_OFF:
            //Make sure all channels are turned on
            ignitionChannelsOn = 0xFFU;
            fuelChannelsOn = 0xFFU;
            currentStatus.engineProtectStatus = 0U;
            break;
          case PROTECT_CUT_IGN:
            ignitionChannelsOn = 0;
            break;
          case PROTECT_CUT_FUEL:
            fuelChannelsOn = 0;
            break;
          case PROTECT_CUT_BOTH:
            ignitionChannelsOn = 0;
            fuelChannelsOn = 0;
            break;
          default:
            ignitionChannelsOn = 0;
            fuelChannelsOn = 0;
            break;
        }
      } //Hard cut check
      else if( (configPage2.hardCutType == HARD_CUT_ROLLING) && (currentStatus.RPM > (maxAllowedRPM + (configPage15.rollingProtRPMDelta[0] * 10))) ) //Limit for rolling is the max allowed RPM minus the lowest value in the delta table (Delta values are negative!)
      { 
        uint8_t revolutionsToCut = 1U;
        if(configPage2.strokes == FOUR_STROKE) { revolutionsToCut *= 2U; } //4 stroke needs to cut for at least 2 revolutions
        if( (configPage4.sparkMode != IGN_MODE_SEQUENTIAL) || (configPage2.injLayout != INJ_SEQUENTIAL) ) { revolutionsToCut *= 2U; } //4 stroke and non-sequential will cut for 4 revolutions minimum. This is to ensure no half fuel ignition cycles take place

        if(rollingCutLastRev == 0U) { rollingCutLastRev = currentStatus.startRevolutions; } //First time check
        if ( (currentStatus.startRevolutions >= (rollingCutLastRev + revolutionsToCut)) || (currentStatus.RPM > maxAllowedRPM) ) //If current RPM is over the max allowed RPM always cut, otherwise check if the required number of revolutions have passed since the last cut
        { 
          uint8_t cutPercent = 0U;
          int16_t rpmDelta = currentStatus.RPM - maxAllowedRPM;
          if(rpmDelta >= 0) { cutPercent = 100U; } //If the current RPM is over the max allowed RPM then cut is full (100%)
          else { cutPercent = table2D_getValue(&rollingCutTable, (rpmDelta / 10) ); } //
          

          for(uint8_t x=0; x<max(maxIgnOutputs, maxInjPrimaryOutputs+maxInjSecondaryOutputs); x++)
          {  
            if( (cutPercent == 100) || (random1to100() < cutPercent) )
            {
              switch(configPage6.engineProtectType)
              {
                case PROTECT_CUT_OFF:
                  //Make sure all channels are turned on
                  ignitionChannelsOn = 0xFFU;
                  fuelChannelsOn = 0xFFU;
                  break;
                case PROTECT_CUT_IGN:
                  BIT_CLEAR(ignitionChannelsOn, x); //Turn off this ignition channel
                  disableScheduleIfPending(ignitionSchedules[x]);
                  break;
                case PROTECT_CUT_FUEL:
                  BIT_CLEAR(fuelChannelsOn, x); //Turn off this fuel channel
                  disableScheduleIfPending(fuelSchedules[x]);
                  break;
                case PROTECT_CUT_BOTH:
                  BIT_CLEAR(ignitionChannelsOn, x); //Turn off this ignition channel
                  BIT_CLEAR(fuelChannelsOn, x); //Turn off this fuel channel
                  disableScheduleIfPending(fuelSchedules[x]);
                  disableScheduleIfPending(ignitionSchedules[x]);
                  break;
                default:
                  BIT_CLEAR(ignitionChannelsOn, x); //Turn off this ignition channel
                  BIT_CLEAR(fuelChannelsOn, x); //Turn off this fuel channel
                  break;
              }
            }
            else
            {
              //Turn fuel and ignition channels on

              //Special case for non-sequential, 4-stroke where both fuel and ignition are cut. The ignition pulses should wait 1 cycle after the fuel channels are turned back on before firing again
              if( (revolutionsToCut == 4U) &&                          //4 stroke and non-sequential
                  (BIT_CHECK(fuelChannelsOn, x) == false) &&          //Fuel on this channel is currently off, meaning it is the first revolution after a cut
                  (configPage6.engineProtectType == PROTECT_CUT_BOTH) //Both fuel and ignition are cut
                )
              { BIT_SET(ignitionChannelsPending, x); } //Set this ignition channel as pending
              else { BIT_SET(ignitionChannelsOn, x); } //Turn on this ignition channel
                
              
              BIT_SET(fuelChannelsOn, x); //Turn on this fuel channel
            }
          }
          rollingCutLastRev = currentStatus.startRevolutions;
        }

        //Check whether there are any ignition channels that are waiting for injection pulses to occur before being turned back on. This can only occur when at least 2 revolutions have taken place since the fuel was turned back on
        //Note that ignitionChannelsPending can only be >0 on 4 stroke, non-sequential fuel when protect type is Both
        if( (ignitionChannelsPending > 0U) && (currentStatus.startRevolutions >= (rollingCutLastRev + 2U)) )
        {
          ignitionChannelsOn = fuelChannelsOn;
          ignitionChannelsPending = 0;
        }
      } //Rolling cut check
      else
      {
        currentStatus.engineProtectStatus = 0;
        //No engine protection active, so turn all the channels on
        if(currentStatus.startRevolutions >= configPage4.StgCycles)
        { 
          //Enable the fuel and ignition, assuming staging revolutions are complete 
          ignitionChannelsOn = 0xff; 
          fuelChannelsOn = 0xff; 
        } 
      }

      if (!BIT_CHECK(currentStatus.status1, BIT_STATUS1_BOOSTCUT))
      {
      //***********************************************************************************************
        setFuelSchedules(currentStatus.injAngle, injectorLimits(getCrankAngle()));
      }

      //***********************************************************************************************
      //| BEGIN IGNITION SCHEDULES
      //Same as above, except for ignition
      if(ignitionChannelsOn)
      {
        setIgnitionSchedules(ignitionLimits(getCrankAngle()), currentStatus.dwell + computeFixedCrankingOverride());
      } //Ignition schedules on

      if ( (!BIT_CHECK(currentStatus.status3, BIT_STATUS3_RESET_PREVENT)) && (resetControl == RESET_CONTROL_PREVENT_WHEN_RUNNING) ) 
      {
        //Reset prevention is supposed to be on while the engine is running but isn't. Fix that.
        digitalWrite(pinResetControl, HIGH);
        BIT_SET(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
      }
    } //Has sync and RPM
    else if ( (BIT_CHECK(currentStatus.status3, BIT_STATUS3_RESET_PREVENT) > 0) && (resetControl == RESET_CONTROL_PREVENT_WHEN_RUNNING) )
    {
      digitalWrite(pinResetControl, LOW);
      BIT_CLEAR(currentStatus.status3, BIT_STATUS3_RESET_PREVENT);
    }
} //loop()
#pragma GCC diagnostic pop

#endif //Unit test guard

/** Calculate the Ignition angles for all cylinders (based on @ref config2.nCylinders).
 * both start and end angles are calculated for each channel.
 * Also the mode of ignition firing - wasted spark vs. dedicated spark per cyl. - is considered here.
 */
static inline void calculateIgnitionAngles(int16_t dwellTime, int8_t advance, int16_t ignLoad)
{
  const uint16_t dwellAngle = timeToAngleDegPerMicroSec(dwellTime); //Convert the dwell time to dwell angle based on the current engine speed

  // Rotary is a special case
  if (configPage2.nCylinders==4 && configPage4.sparkMode == IGN_MODE_ROTARY)
  {
    int16_t splitDegrees = table2D_getValue(&rotarySplitTable, ignLoad);

    calculateIgnitionAngles(ignitionSchedules[0], dwellAngle, advance);
    calculateIgnitionAngles(ignitionSchedules[1], dwellAngle, advance);
    //The trailing angles are set relative to the leading ones
    calculateIgnitionTrailingRotary(ignitionSchedules[2], dwellAngle, splitDegrees, ignitionSchedules[0]);
    calculateIgnitionTrailingRotary(ignitionSchedules[3], dwellAngle, splitDegrees, ignitionSchedules[1]);
  }
  else
  {
    bool supportsSequential =    (_countof(ignitionSchedules)>=configPage2.nCylinders)
                              && (configPage2.nCylinders==4 || configPage2.nCylinders==6 || configPage2.nCylinders==8);
    if (supportsSequential)
    {
      if((configPage4.sparkMode==IGN_MODE_SEQUENTIAL) && currentStatus.hasSync)
      {
        if (maxIgnOutputs!=configPage2.nCylinders) { changeHalfToFullSync(); } 
      }
      else 
      {
        if( BIT_CHECK(currentStatus.status3, BIT_STATUS3_HALFSYNC) && (maxIgnOutputs==configPage2.nCylinders)) { changeFullToHalfSync(); }
      }
    }

    for (uint8_t index=0; index < maxIgnOutputs; ++index) {
      calculateIgnitionAngles(ignitionSchedules[index], dwellAngle, advance);
    }
  }

  // If ignition timing is being tracked per tooth, perform the calcs to get the end teeth
  // This relies on the ignition angles, so can only be called after those are calculated.
  if( (configPage2.perToothIgn == true) ) { triggerSetEndTeeth(); }
}

TESTABLE_INLINE_STATIC void calculateStaging(uint16_t pwLimit, uint16_t pwPrimary)
{
  // Later code relies on pw==0 indicating an excluded/unused channel
  for (uint8_t index=0; index<_countof(fuelSchedules); ++index) {
    fuelSchedules[index].pw = 0U;
  }

  uint16_t pwSecondary = 0U;
  //Calculate staging pulsewidths if used
  //To run staged injection, the number of cylinders must be less than or equal to the injector channels (ie Assuming you're running paired injection, you need at least as many injector channels as you have cylinders, half for the primaries and half for the secondaries)
  if( (configPage10.stagingEnabled == true) && (configPage2.nCylinders <= (uint8_t)INJ_CHANNELS || configPage2.injType == INJ_TYPE_TBODY) && (pwPrimary > inj_opentime_uS) ) //Final check is to ensure that DFCO isn't active, which would cause an overflow below (See #267)
  {
    //Scale the 'full' pulsewidth by each of the injector capacities
    pwPrimary -= inj_opentime_uS; //Subtract the opening time from PW1 as it needs to be multiplied out again by the pri/sec req_fuel values below. It is added on again after that calculation. 
    uint32_t tempPW1 = div100((uint32_t)pwPrimary * staged_req_fuel_mult_pri);

    if(configPage10.stagingMode == STAGING_MODE_TABLE)
    {
      uint32_t tempPW3 = div100((uint32_t)pwPrimary * staged_req_fuel_mult_sec); //This is ONLY needed in in table mode. Auto mode only calculates the difference.

      byte stagingSplit = get3DTableValue(&stagingTable, currentStatus.fuelLoad, currentStatus.RPM);
      pwPrimary = div100((100 - stagingSplit) * tempPW1);
      pwPrimary += inj_opentime_uS; 

      //PW2 is used temporarily to hold the secondary injector pulsewidth. It will be assigned to the correct channel below
      if(stagingSplit > 0) 
      { 
        BIT_SET(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE); //Set the staging active flag
        pwSecondary = div100(stagingSplit * tempPW3); 
        pwSecondary += inj_opentime_uS;
      }
      else
      {
        BIT_CLEAR(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE); //Clear the staging active flag
        pwSecondary = 0; 
      }
    }
    else if(configPage10.stagingMode == STAGING_MODE_AUTO)
    {
      pwPrimary = tempPW1;
      //If automatic mode, the primary injectors are used all the way up to their limit (Configured by the pulsewidth limit setting)
      //If they exceed their limit, the extra duty is passed to the secondaries
      if(tempPW1 > pwLimit)
      {
        BIT_SET(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE); //Set the staging active flag
        uint32_t extraPW = tempPW1 - pwLimit + inj_opentime_uS; //The open time must be added here AND below because tempPW1 does not include an open time. The addition of it here takes into account the fact that pwLlimit does not contain an allowance for an open time. 
        pwPrimary = pwLimit;
        pwSecondary = udiv_32_16(extraPW * staged_req_fuel_mult_sec, staged_req_fuel_mult_pri); //Convert the 'left over' fuel amount from primary injector scaling to secondary
        pwSecondary += inj_opentime_uS;
      }
      else 
      {
        //If tempPW1 < pwLImit it means that the entire fuel load can be handled by the primaries and staging is inactive. 
        pwPrimary += inj_opentime_uS; //Add the open time back in
        BIT_CLEAR(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE); //Clear the staging active flag 
        pwSecondary = 0; 
      } 
    }
  } else {
      //Apply the pwLimit if staging is disabled and engine is not cranking
      if( (!BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) ) { 
        pwPrimary = min(pwPrimary, pwLimit); 
      }
  }

  if (pwSecondary>0) {
    fuelSchedules[0].pw = pwPrimary;
    //Allocate the primary and secondary pulse widths based on the fuel configuration
    switch (configPage2.nCylinders) 
    {
      case 1:
        //Nothing required for 1 cylinder, channels are correct already
        break;
      case 2:
        //Primary pulsewidth on channels 1 and 2, secondary on channels 3 and 4
        fuelSchedules[2].pw = pwSecondary;
        fuelSchedules[3].pw = pwSecondary;
        fuelSchedules[1].pw = pwPrimary;
        break;
      case 3:
        //6 channels required for 'normal' 3 cylinder staging support
        #if INJ_CHANNELS >= 6
          //Primary pulsewidth on channels 1, 2 and 3, secondary on channels 4, 5 and 6
          fuelSchedules[3].pw = pwSecondary;
          fuelSchedules[4].pw = pwSecondary;
          fuelSchedules[5].pw = pwSecondary;
        #else
          //If there are not enough channels, then primary pulsewidth is on channels 1, 2 and 3, secondary on channel 4
          fuelSchedules[3].pw = pwSecondary;
        #endif
        fuelSchedules[1].pw = pwPrimary;
        fuelSchedules[2].pw = pwPrimary;
        break;
      case 4:
        if( (configPage2.injLayout == INJ_SEQUENTIAL) || (configPage2.injLayout == INJ_SEMISEQUENTIAL) )
        {
          //Staging with 4 cylinders semi/sequential requires 8 total channels
          #if INJ_CHANNELS >= 8
            fuelSchedules[4].pw = pwSecondary;
            fuelSchedules[5].pw = pwSecondary;
            fuelSchedules[6].pw = pwSecondary;
            fuelSchedules[7].pw = pwSecondary;

            fuelSchedules[1].pw = pwPrimary;
            fuelSchedules[2].pw = pwPrimary;
            fuelSchedules[3].pw = pwPrimary;
          #else
            //This is an invalid config as there are not enough outputs to support sequential + staging
            //Put the staging output to the non-existant channel 5
#if INJ_CHANNELS >= 5            
            fuelSchedules[4].pw = pwSecondary;
#endif
          #endif
        }
        else
        {
          fuelSchedules[2].pw = pwSecondary;
          fuelSchedules[3].pw = pwSecondary;
          fuelSchedules[1].pw = pwPrimary;
        }
        break;
        
      case 5:
        //No easily supportable 5 cylinder staging option unless there are at least 5 channels
        #if INJ_CHANNELS >= 5
          if (configPage2.injLayout != INJ_SEQUENTIAL)
          {
            fuelSchedules[4].pw = pwSecondary;
          }
          #if INJ_CHANNELS >= 6
            fuelSchedules[5].pw = pwSecondary;
          #endif
        #endif
        
          fuelSchedules[1].pw = pwPrimary;
          fuelSchedules[2].pw = pwPrimary;
          fuelSchedules[3].pw = pwPrimary;
        break;

      case 6:
        #if INJ_CHANNELS >= 6
          //8 cylinder staging only if not sequential
          if (configPage2.injLayout != INJ_SEQUENTIAL)
          {
            fuelSchedules[3].pw = pwSecondary;
            fuelSchedules[4].pw = pwSecondary;
            fuelSchedules[5].pw = pwSecondary;
          }
          #if INJ_CHANNELS >= 8
          else
            {
              //If there are 8 channels, then the 6 cylinder sequential option is available by using channels 7 + 8 for staging
              fuelSchedules[6].pw = pwSecondary;
              fuelSchedules[7].pw = pwSecondary;

              fuelSchedules[3].pw = pwPrimary;
              fuelSchedules[4].pw = pwPrimary;
              fuelSchedules[5].pw = pwPrimary;
            }
          #endif
        #endif
        fuelSchedules[1].pw = pwPrimary;
        fuelSchedules[2].pw = pwPrimary;
        break;

      case 8:
        #if INJ_CHANNELS >= 8
          //8 cylinder staging only if not sequential
          if (configPage2.injLayout != INJ_SEQUENTIAL)
          {
            fuelSchedules[4].pw = pwSecondary;
            fuelSchedules[5].pw = pwSecondary;
            fuelSchedules[6].pw = pwSecondary;
            fuelSchedules[7].pw = pwSecondary;
          }
        #endif
        fuelSchedules[1].pw = pwPrimary;
        fuelSchedules[2].pw = pwPrimary;
        fuelSchedules[3].pw = pwPrimary;
        break;

      default:
        //Assume 4 cylinder non-seq for default
        fuelSchedules[2].pw = pwSecondary;
        fuelSchedules[3].pw = pwSecondary;
        fuelSchedules[1].pw = pwPrimary;
        break;
    }
  }
  else 
  { 
    //If staging is off, all the pulse widths are set the same (Sequential and other adjustments may be made below)
    for (uint8_t index=0U; index<maxInjPrimaryOutputs; ++index) {
      fuelSchedules[index].pw = pwPrimary;
    }

    BIT_CLEAR(currentStatus.status4, BIT_STATUS4_STAGING_ACTIVE); //Clear the staging active flag    
  } 
}

void checkLaunchAndFlatShift(void)
{
  //Check for launching/flat shift (clutch) based on the current and previous clutch states
  currentStatus.previousClutchTrigger = currentStatus.clutchTrigger;
  //Only check for pinLaunch if any function using it is enabled. Else pins might break starting a board
  if(configPage6.flatSEnable || configPage6.launchEnabled)
  {
    if(configPage6.launchHiLo > 0) { currentStatus.clutchTrigger = digitalRead(pinLaunch); }
    else { currentStatus.clutchTrigger = !digitalRead(pinLaunch); }
  }
  if(currentStatus.clutchTrigger && (currentStatus.previousClutchTrigger != currentStatus.clutchTrigger) ) { currentStatus.clutchEngagedRPM = currentStatus.RPM; } //Check whether the clutch has been engaged or disengaged and store the current RPM if so

  //Default flags to off
  currentStatus.launchingHard = false; 
  BIT_CLEAR(currentStatus.status2, BIT_STATUS2_HLAUNCH); 
  currentStatus.flatShiftingHard = false;

  if (configPage6.launchEnabled && currentStatus.clutchTrigger && (currentStatus.clutchEngagedRPM < ((unsigned int)(configPage6.flatSArm) * 100)) && (currentStatus.TPS >= configPage10.lnchCtrlTPS) ) 
  { 
    //If VSS is used, make sure we're not above the speed limit
    if ( (configPage2.vssMode > 0) && (currentStatus.vss >= configPage10.lnchCtrlVss) )
    {
      return;
    }
    
    //Check whether RPM is above the launch limit
    uint16_t launchRPMLimit = (configPage6.lnchHardLim * 100);
    if( (configPage2.hardCutType == HARD_CUT_ROLLING) ) { launchRPMLimit += (configPage15.rollingProtRPMDelta[0] * 10); } //Add the rolling cut delta if enabled (Delta is a negative value)

    if(currentStatus.RPM > launchRPMLimit)
    {
      //HardCut rev limit for 2-step launch control.
      currentStatus.launchingHard = true; 
      BIT_SET(currentStatus.status2, BIT_STATUS2_HLAUNCH); 
    }
  } 
  else 
  { 
    //If launch is not active, check whether flat shift should be active
    if(configPage6.flatSEnable && currentStatus.clutchTrigger && (currentStatus.clutchEngagedRPM >= ((unsigned int)(configPage6.flatSArm * 100)) ) ) 
    { 
      uint16_t flatRPMLimit = currentStatus.clutchEngagedRPM;
      if( (configPage2.hardCutType == HARD_CUT_ROLLING) ) { flatRPMLimit += (configPage15.rollingProtRPMDelta[0] * 10); } //Add the rolling cut delta if enabled (Delta is a negative value)

      if(currentStatus.RPM > flatRPMLimit)
      {
        //Flat shift rev limit
        currentStatus.flatShiftingHard = true;
      }
    }
  }
}
