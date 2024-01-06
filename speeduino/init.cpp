/** @file
 * Speeduino Initialisation (called at Arduino setup()).
 */
#include "globals.h"
#include "init.h"
#include "storage.h"
#include "updates.h"
#include "speeduino.h"
#include "timers.h"
#include "comms_secondary.h"
#include "comms_CAN.h"
#include "utilities.h"
#include "scheduler.h"
#include "auxiliaries.h"
#include "sensors.h"
#include "decoders.h"
#include "corrections.h"
#include "idle.h"
#include "table2d.h"
#include "decoder_triggers.h"
#include "pin_mapping.h"
#include "secondaryTables.h"
#if defined(EEPROM_RESET_PIN)
#include EEPROM_LIB_H
#endif
#include "SD_logger.h"
#include "rtc_common.h"

static void configure2dTables(void) {
  //Repoint the 2D table structs to the config pages that were just loaded
  taeTable.valueSize = SIZE_BYTE; //Set this table to use byte values
  taeTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  taeTable.xSize = 4;
  taeTable.values = configPage4.taeValues;
  taeTable.axisX = configPage4.taeBins;
  
  maeTable.valueSize = SIZE_BYTE; //Set this table to use byte values
  maeTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  maeTable.xSize = 4;
  maeTable.values = configPage4.maeRates;
  maeTable.axisX = configPage4.maeBins;
  
  WUETable.valueSize = SIZE_BYTE; //Set this table to use byte values
  WUETable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  WUETable.xSize = 10;
  WUETable.values = configPage2.wueValues;
  WUETable.axisX = configPage4.wueBins;
  
  ASETable.valueSize = SIZE_BYTE;
  ASETable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  ASETable.xSize = 4;
  ASETable.values = configPage2.asePct;
  ASETable.axisX = configPage2.aseBins;
  
  ASECountTable.valueSize = SIZE_BYTE;
  ASECountTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  ASECountTable.xSize = 4;
  ASECountTable.values = configPage2.aseCount;
  ASECountTable.axisX = configPage2.aseBins;
  
  PrimingPulseTable.valueSize = SIZE_BYTE;
  PrimingPulseTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  PrimingPulseTable.xSize = 4;
  PrimingPulseTable.values = configPage2.primePulse;
  PrimingPulseTable.axisX = configPage2.primeBins;
  
  crankingEnrichTable.valueSize = SIZE_BYTE;
  crankingEnrichTable.axisSize = SIZE_BYTE;
  crankingEnrichTable.xSize = 4;
  crankingEnrichTable.values = configPage10.crankingEnrichValues;
  crankingEnrichTable.axisX = configPage10.crankingEnrichBins;

  dwellVCorrectionTable.valueSize = SIZE_BYTE;
  dwellVCorrectionTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  dwellVCorrectionTable.xSize = 6;
  dwellVCorrectionTable.values = configPage4.dwellCorrectionValues;
  dwellVCorrectionTable.axisX = configPage6.voltageCorrectionBins;
  
  injectorVCorrectionTable.valueSize = SIZE_BYTE;
  injectorVCorrectionTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  injectorVCorrectionTable.xSize = 6;
  injectorVCorrectionTable.values = configPage6.injVoltageCorrectionValues;
  injectorVCorrectionTable.axisX = configPage6.voltageCorrectionBins;
  
  injectorAngleTable.valueSize = SIZE_INT;
  injectorAngleTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  injectorAngleTable.xSize = 4;
  injectorAngleTable.values = configPage2.injAng;
  injectorAngleTable.axisX = configPage2.injAngRPM;
  
  IATDensityCorrectionTable.valueSize = SIZE_BYTE;
  IATDensityCorrectionTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  IATDensityCorrectionTable.xSize = 9;
  IATDensityCorrectionTable.values = configPage6.airDenRates;
  IATDensityCorrectionTable.axisX = configPage6.airDenBins;
  
  baroFuelTable.valueSize = SIZE_BYTE;
  baroFuelTable.axisSize = SIZE_BYTE;
  baroFuelTable.xSize = 8;
  baroFuelTable.values = configPage4.baroFuelValues;
  baroFuelTable.axisX = configPage4.baroFuelBins;
  
  IATRetardTable.valueSize = SIZE_BYTE;
  IATRetardTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  IATRetardTable.xSize = 6;
  IATRetardTable.values = configPage4.iatRetValues;
  IATRetardTable.axisX = configPage4.iatRetBins;
  
  CLTAdvanceTable.valueSize = SIZE_BYTE;
  CLTAdvanceTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  CLTAdvanceTable.xSize = 6;
  CLTAdvanceTable.values = (byte*)configPage4.cltAdvValues;
  CLTAdvanceTable.axisX = configPage4.cltAdvBins;
  
  idleTargetTable.valueSize = SIZE_BYTE;
  idleTargetTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  idleTargetTable.xSize = 10;
  idleTargetTable.values = configPage6.iacCLValues;
  idleTargetTable.axisX = configPage6.iacBins;
  
  idleAdvanceTable.valueSize = SIZE_BYTE;
  idleAdvanceTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  idleAdvanceTable.xSize = 6;
  idleAdvanceTable.values = (byte*)configPage4.idleAdvValues;
  idleAdvanceTable.axisX = configPage4.idleAdvBins;
  
  rotarySplitTable.valueSize = SIZE_BYTE;
  rotarySplitTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  rotarySplitTable.xSize = 8;
  rotarySplitTable.values = configPage10.rotarySplitValues;
  rotarySplitTable.axisX = configPage10.rotarySplitBins;

  flexFuelTable.valueSize = SIZE_BYTE;
  flexFuelTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  flexFuelTable.xSize = 6;
  flexFuelTable.values = configPage10.flexFuelAdj;
  flexFuelTable.axisX = configPage10.flexFuelBins;
  
  flexAdvTable.valueSize = SIZE_BYTE;
  flexAdvTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  flexAdvTable.xSize = 6;
  flexAdvTable.values = configPage10.flexAdvAdj;
  flexAdvTable.axisX = configPage10.flexAdvBins;

  flexBoostTable.valueSize = SIZE_INT;
  flexBoostTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins (NOTE THIS IS DIFFERENT TO THE VALUES!!)
  flexBoostTable.xSize = 6;
  flexBoostTable.values = configPage10.flexBoostAdj;
  flexBoostTable.axisX = configPage10.flexBoostBins;

  fuelTempTable.valueSize = SIZE_BYTE;
  fuelTempTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  fuelTempTable.xSize = 6;
  fuelTempTable.values = configPage10.fuelTempValues;
  fuelTempTable.axisX = configPage10.fuelTempBins;

  knockWindowStartTable.valueSize = SIZE_BYTE;
  knockWindowStartTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  knockWindowStartTable.xSize = 6;
  knockWindowStartTable.values = configPage10.knock_window_angle;
  knockWindowStartTable.axisX = configPage10.knock_window_rpms;

  knockWindowDurationTable.valueSize = SIZE_BYTE;
  knockWindowDurationTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  knockWindowDurationTable.xSize = 6;
  knockWindowDurationTable.values = configPage10.knock_window_dur;
  knockWindowDurationTable.axisX = configPage10.knock_window_rpms;

  oilPressureProtectTable.valueSize = SIZE_BYTE;
  oilPressureProtectTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  oilPressureProtectTable.xSize = 4;
  oilPressureProtectTable.values = configPage10.oilPressureProtMins;
  oilPressureProtectTable.axisX = configPage10.oilPressureProtRPM;

  coolantProtectTable.valueSize = SIZE_BYTE;
  coolantProtectTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  coolantProtectTable.xSize = 6;
  coolantProtectTable.values = configPage9.coolantProtRPM;
  coolantProtectTable.axisX = configPage9.coolantProtTemp;

  fanPWMTable.valueSize = SIZE_BYTE;
  fanPWMTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  fanPWMTable.xSize = 4;
  fanPWMTable.values = configPage9.PWMFanDuty;
  fanPWMTable.axisX = configPage6.fanPWMBins;

  rollingCutTable.valueSize = SIZE_BYTE;
  rollingCutTable.axisSize = SIZE_SIGNED_BYTE; //X axis is SIGNED for this table. 
  rollingCutTable.xSize = 4;
  rollingCutTable.values = configPage15.rollingProtCutPercent;
  rollingCutTable.axisX = configPage15.rollingProtRPMDelta;
  
  wmiAdvTable.valueSize = SIZE_BYTE;
  wmiAdvTable.axisSize = SIZE_BYTE; //Set this table to use byte axis bins
  wmiAdvTable.xSize = 6;
  wmiAdvTable.values = configPage10.wmiAdvAdj;
  wmiAdvTable.axisX = configPage10.wmiAdvBins;

  cltCalibrationTable.valueSize = SIZE_INT;
  cltCalibrationTable.axisSize = SIZE_INT;
  cltCalibrationTable.xSize = 32;
  cltCalibrationTable.values = cltCalibration_values;
  cltCalibrationTable.axisX = cltCalibration_bins;

  iatCalibrationTable.valueSize = SIZE_INT;
  iatCalibrationTable.axisSize = SIZE_INT;
  iatCalibrationTable.xSize = 32;
  iatCalibrationTable.values = iatCalibration_values;
  iatCalibrationTable.axisX = iatCalibration_bins;

  o2CalibrationTable.valueSize = SIZE_BYTE;
  o2CalibrationTable.axisSize = SIZE_INT;
  o2CalibrationTable.xSize = 32;
  o2CalibrationTable.values = o2Calibration_values;
  o2CalibrationTable.axisX = o2Calibration_bins;
}

static void calculateRequiredFuel(void) {
  req_fuel_uS = configPage2.reqFuel * 100; //Convert to uS and an int. This is the only variable to be used in calculations
  inj_opentime_uS = configPage2.injOpen * 100; //Injector open time. Comes through as ms*10 (Eg 15.5ms = 155).

  if(configPage10.stagingEnabled == true)
  {
    uint32_t totalInjector = configPage10.stagedInjSizePri + configPage10.stagedInjSizeSec;
    /*
        These values are a percentage of the req_fuel value that would be required for each injector channel to deliver that much fuel.
        Eg:
        Pri injectors are 250cc
        Sec injectors are 500cc
        Total injector capacity = 750cc

        staged_req_fuel_mult_pri = 300% (The primary injectors would have to run 3x the overall PW in order to be the equivalent of the full 750cc capacity
        staged_req_fuel_mult_sec = 150% (The secondary injectors would have to run 1.5x the overall PW in order to be the equivalent of the full 750cc capacity
    */
    staged_req_fuel_mult_pri = (100 * totalInjector) / configPage10.stagedInjSizePri;
    staged_req_fuel_mult_sec = (100 * totalInjector) / configPage10.stagedInjSizeSec;
  }
}

static void checkForEepromReset(void) {
#if defined(EEPROM_RESET_PIN)
  uint32_t start_time = millis();
  byte exit_erase_loop = false; 
  pinMode(EEPROM_RESET_PIN, INPUT_PULLUP);  

  //only start routine when this pin is low because it is pulled low
  while (digitalRead(EEPROM_RESET_PIN) != HIGH && (millis() - start_time)<1050)
  {
    //make sure the key is pressed for at least 0.5 second 
    if ((millis() - start_time)>500) {
      //if key is pressed afterboot for 0.5 second make led turn off
      digitalWrite(LED_BUILTIN, HIGH);

      //see if the user reacts to the led turned off with removing the keypress within 1 second
      while (((millis() - start_time)<1000) && (exit_erase_loop!=true)){

        //if user let go of key within 1 second erase eeprom
        if(digitalRead(EEPROM_RESET_PIN) != LOW){
          #if defined(FLASH_AS_EEPROM_h)
            EEPROM.read(0); //needed for SPI eeprom emulation.
            EEPROM.clear(); 
          #else 
            for (int i = 0 ; i < EEPROM.length() ; i++) { EEPROM.write(i, 255);}
          #endif
          //if erase done exit while loop.
          exit_erase_loop = true;
        }
      }
    } 
  }
  #endif
}

static void initialiseCurrentStatus(void) {
  currentStatus.fpPrimed = false;
  currentStatus.injPrimed = false;
  currentStatus.RPM = 0;
  currentStatus.hasSync = false;
  BIT_CLEAR(currentStatus.status3, BIT_STATUS3_HALFSYNC);
  currentStatus.runSecs = 0;
  currentStatus.secl = 0;
  //currentStatus.seclx10 = 0;
  currentStatus.startRevolutions = 0;
  currentStatus.syncLossCounter = 0;
  currentStatus.flatShiftingHard = false;
  currentStatus.launchingHard = false;
  currentStatus.crankRPM = ((unsigned int)configPage4.crankRPM * 10); //Crank RPM limit (Saves us calculating this over and over again. It's updated once per second in timers.ino)
  currentStatus.engineProtectStatus = 0;
  currentStatus.nChannels = ((uint8_t)INJ_CHANNELS << 4) + IGN_CHANNELS; //First 4 bits store the number of injection channels, 2nd 4 store the number of ignition channels
}

/** Initialise Speeduino for the main loop.
 * Top level init entry point for all initialisations:
 * - Initialise and set sizes of 3D tables
 * - Load config from EEPROM, update config structures to current version of SW if needed.
 * - Initialise board (The initBoard() is for board X implemented in board_X.ino file)
 * - Initialise timers (See timers.ino)
 * - Perform optional SD card and RTC battery inits
 * - Load calibration tables from EEPROM
 * - Perform pin mapping (calling @ref setPinMapping() based on @ref config2.pinMapping)
 * - Stop any coil charging and close injectors
 * - Initialise schedulers, Idle, Fan, auxPWM, Corrections, AD-conversions, Programmable I/O
 * - Initialise baro (ambient pressure) by reading MAP (before engine runs)
 * - Initialise triggers
 * - Perform cyl. count based initialisations (@ref config2.nCylinders)
 * - Perform injection and spark mode based setup
 *   - Assign injector open/close and coil charge begin/end functions to their dedicated global vars
 * - Perform fuel pressure priming by turning fuel pump on
 * - Read CLT and TPS sensors to have cranking pulsewidths computed correctly
 * - Mark Initialisation completed (this flag-marking is used in code to prevent after-init changes)
 */
void initialiseAll(void)
{   
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    checkForEepromReset();
  
    configure2dTables();

    // Unit tests should be independent of any stored configuration on the board!
#if !defined(UNIT_TEST)
    loadConfig();
    doUpdates(); //Check if any data items need updating (Occurs with firmware updates)
    //Setup the calibration tables
    loadCalibration();
#endif

    // Always start with a clean slate on the bootloader capabilities level
    // This should be 0 until we hear otherwise from the 16u2
    configPage4.bootloaderCaps = 0;
    
    initBoard(); //This calls the current individual boards init function. See the board_xxx.ino files for these.
    
    Serial.begin(115200);
    BIT_SET(currentStatus.status4, BIT_STATUS4_ALLOW_LEGACY_COMMS); //Flag legacy comms as being allowed on startup

    //Set the pin mappings
    if((configPage2.pinMapping == 255) || (configPage2.pinMapping == 0)) //255 = EEPROM value in a blank AVR; 0 = EEPROM value in new FRAM
    {
      //First time running on this board
      resetConfigPages();
      configPage4.triggerTeeth = 4; //Avoiddiv by 0 when start decoders
      setPinMapping(3); //Force board to v0.4
    }
    else { setPinMapping(configPage2.pinMapping); }

    // initialiseAll can be repeatedly called by unit tests. The CAN 
    // library will hang semi-randomly if re-initialized.
    #if defined (NATIVE_CAN_AVAILABLE) && !defined(UNIT_TEST) 
      initCAN();
    #endif

    //Must come after setPinMapping() as secondary serial can be changed on a per board basis
    #if defined(secondarySerial_AVAILABLE)
      if (configPage9.enable_secondarySerial == 1) { secondarySerial.begin(115200); }
    #endif

    //Perform all initialisations
    //initialiseDisplay();
  #ifdef SD_LOGGING
    initRTC();
    initSD(pinMapping);
  #endif
    initialiseIdle(true, pinMapping);
    initialiseFan(pinMapping);
    initialiseAirCon(pinMapping);
    initialiseAuxPWM(pinMapping);
    initialiseCorrections();
    initialiseADC(pinMapping);
    initialiseProgrammableIO(pinMapping);
    initialiseResetControl(pinMapping);
    initialiseIgnitionByPass(pinMapping);
    initialiseLaunchControl(pinMapping);
    initialiseMapBaroSensors(pinMapping);
    initialiseTPS(pinMapping);
    initialiseCoreSensors(pinMapping);
    initialiseNonCoreSensors(pinMapping);
    initialiseFuelPump(pinMapping);
    initialiseSecondaryTables(pinMapping);
    initialiseWmi(pinMapping);

    //Once the configs have been loaded, a number of one time calculations can be completed
    calculateRequiredFuel();

    //Begin the main crank trigger interrupt pin setup
    //The interrupt numbering is a bit odd - See here for reference: arduino.cc/en/Reference/AttachInterrupt
    //These assignments are based on the Arduino Mega AND VARY BETWEEN BOARDS. Please confirm the board you are using and update accordingly.
    initialiseCurrentStatus();
    ms_counter = 0;
    toothHistoryIndex = 0;

    noInterrupts();
    initialiseDecoder();

    // Post trigger initialization we can check if the trigger secondary
    // trigger pins are available for use.
    if ((pinMapping.inputs.pinVSS == pinMapping.inputs.pinTrigger2)
      && BIT_CHECK(decoderState, BIT_DECODER_HAS_SECONDARY)) {
      pinMapping.inputs.pinVSS = NOT_A_PIN;
    }
    initialiseVss(pinMapping);
    if ((pinMapping.inputs.pinFlex == pinMapping.inputs.pinTrigger2)
      && BIT_CHECK(decoderState, BIT_DECODER_HAS_SECONDARY)) {
        pinMapping.inputs.pinFlex = NOT_A_PIN;
    }
    initialiseFlexFuel(pinMapping);

    //End crank trigger interrupt attachment

    //Initial values for loop times
    currentLoopTime = micros_safe();
    mainLoopCount = 0;
    
    initialiseFuelSchedulers(pinMapping);
    initialiseIgnitionSchedulers(pinMapping);

    interrupts();
    readCLT(false); // Need to read coolant temp to make priming pulsewidth work correctly. The false here disables use of the filter
    readTPS(false); // Need to read tps to detect flood clear state

    initialiseTimers(pinMapping);

    currentStatus.initialisationComplete = true;
    digitalWrite(LED_BUILTIN, HIGH);
}
