#pragma once

#include <stdint.h>
#include "board_definition.h"

struct output_pins_t {
  uint8_t pinInjectors[INJ_CHANNELS];
  uint8_t pinCoils[IGN_CHANNELS];
  uint8_t pinFuelPump;
  uint8_t pinTachOut;
  uint8_t pinFan;
  uint8_t pinResetControl;
  uint8_t pinIdle1;
  uint8_t pinIdle2;
  uint8_t pinIdleUpOutput;
  uint8_t pinStepperDir;
  uint8_t pinStepperStep;
  uint8_t pinStepperEnable;
  uint8_t pinBoost;
  uint8_t pinVVT_1;
  uint8_t pinVVT_2;
  uint8_t pinIgnBypass;
  uint8_t pinWMIEnabled;
  uint8_t pinWMIIndicator;
  uint8_t pinAirConComp;
  uint8_t pinAirConFan;
#if defined(OUTPUT_CONTROL_SUPPORTED)
  uint8_t pinMC33810_1;
  uint8_t pinMC33810_2;
#endif  
#if defined(ENABLE_SPARE_OUTPUT)
  uint8_t pinSpareOut1;
  uint8_t pinSpareOut2;
  uint8_t pinSpareOut3;
  uint8_t pinSpareOut4;
  uint8_t pinSpareOut5;
  uint8_t pinSpareOut6;
#endif
#if defined(ENABLE_SPARE_HIGH_OUTPUT)
  uint8_t pinSpareHOut1;
  uint8_t pinSpareHOut2;
#endif
#if defined(ENABLE_SPARE_LOW_OUTPUT)
  uint8_t pinSpareLOut1;
  uint8_t pinSpareLOut2;
  uint8_t pinSpareLOut3;
  uint8_t pinSpareLOut4;
  uint8_t pinSpareLOut5;
#endif
};

// Analog inputs
struct sensor_pins_t {
  uint8_t pinCLT; 
  uint8_t pinIAT;
  uint8_t pinMAP;
  #if defined(USE_MAP2)
  uint8_t pinMAP2;
  #endif
  uint8_t pinEMAP;
  uint8_t pinTPS;
  uint8_t pinO2;
  uint8_t pinO2_2;
  uint8_t pinBat;
  uint8_t pinBaro;
  uint8_t pinFuelPressure;
  uint8_t pinOilPressure;
};

struct input_pins_t {
  sensor_pins_t sensors;
  uint8_t pinTrigger;
  uint8_t pinTrigger2;
  uint8_t pinTrigger3;
#if defined(ENABLE_DISPLAY_RESET)
  uint8_t pinDisplayReset;
#endif
  uint8_t pinIdleUp;
  uint8_t pinCTPS;
  uint8_t pinFuel2Input;
  uint8_t pinSpark2Input;
  uint8_t pinLaunch;
  uint8_t pinVSS;
  uint8_t pinWMIEmpty;
  uint8_t pinFlex;
#ifdef SD_LOGGING  
  uint8_t pinSDEnable;
#endif
  uint8_t pinAirConRequest;
#ifdef USE_SPI_EEPROM
  uint8_t pinSPIFlash_CS;
#endif
#if defined(ENABLE_SPARE_TEMP)
  uint8_t pinSpareTemp1;
  uint8_t pinSpareTemp2;
#endif
};

struct pin_mapping_t {
    output_pins_t outputs;
    input_pins_t inputs;
};

extern pin_mapping_t pinMapping;

void setPinMapping(uint8_t boardId);
bool pinIsUsed(uint8_t pin, const pin_mapping_t &pins);
