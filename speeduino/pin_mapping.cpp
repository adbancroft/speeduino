#include "globals.h"
#include "pin_mapping.h"
#include "utilities.h"
#include "acc_mc33810.h"
#include "idle.h"
#include "sensors.h"
#include "auxiliaries.h"
#include "SD_logger.h"
#include "utilities.h"

static constexpr uint8_t NUM_PINS = sizeof(pin_mapping_t)/sizeof(uint8_t);
static constexpr uint8_t NUM_OUTPUT_PINS = sizeof(output_pins_t)/sizeof(uint8_t);
static constexpr uint8_t NUM_INDIVIDUAL_OUTPUT_PINS = (uint8_t)((sizeof(output_pins_t)-sizeof(output_pins_t::pinInjectors)-sizeof(output_pins_t::pinCoils))/sizeof(uint8_t));
static constexpr uint8_t NUM_INPUT_PINS = sizeof(input_pins_t)/sizeof(uint8_t);
static constexpr uint8_t NUM_SENSOR_PINS = sizeof(sensor_pins_t)/sizeof(uint8_t);

#ifndef SMALL_FLASH_MODE //No support for bluepill here anyway

static pin_mapping_t getPinMappingsV2_0Shield(void) {
  pin_mapping_t boardDefaultPins = { };
      //Pin mappings as per the v0.2 shield
  static const uint8_t boardFuelPins[] PROGMEM = { 8, 9, 10, 11, 12 };
  static const uint8_t boardCoilPins[] PROGMEM = { 28, 24, 40, 36, 34 };
    memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
    memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 20; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 21; //The Cam Sensor pin
  boardDefaultPins.inputs.pinTrigger3 = 3; //The Cam sensor 2 pin
  boardDefaultPins.inputs.sensors.pinTPS = A2; //TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A3; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A0; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A1; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A8; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A4; //Battery reference voltage pin
#if defined(ENABLE_DISPLAY_RESET)
  boardDefaultPins.inputs.pinDisplayReset = 48; // OLED reset pin
#endif
  boardDefaultPins.outputs.pinTachOut = 49; //Tacho output pin
  boardDefaultPins.outputs.pinIdle1 = 30; //Single wire idle control
  boardDefaultPins.outputs.pinIdle2 = 31; //2 wire idle control
  boardDefaultPins.outputs.pinStepperDir = 16; //Direction pin  for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = 17; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinFan = 47; //Pin for the fan output
  boardDefaultPins.outputs.pinFuelPump = 4; //Fuel pump output
  boardDefaultPins.inputs.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
  boardDefaultPins.outputs.pinResetControl = 43; //Reset control output

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingsV3_0Shield(void) {
  pin_mapping_t boardDefaultPins = { };
      //Pin mappings as per the v0.3 shield
#if defined(CORE_TEENSY35)
  static const uint8_t boardFuelPins[] PROGMEM = { 8, 9, 10, 11, 12 };
  static const uint8_t boardCoilPins[] PROGMEM = { 31, 24, 30, 21, 34 };
#else
  static const uint8_t boardFuelPins[] PROGMEM = { 8, 9, 10, 11, 12 };
  static const uint8_t boardCoilPins[] PROGMEM = { 28, 24, 40, 36, 34 };
#endif
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 19; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 18; //The Cam Sensor pin
  boardDefaultPins.inputs.pinTrigger3 = 3; //The Cam sensor 2 pin
  boardDefaultPins.inputs.sensors.pinTPS = A2;//TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A3; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A0; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A1; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A8; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A4; //Battery reference voltage pin
#if defined(ENABLE_DISPLAY_RESET)
  boardDefaultPins.inputs.pinDisplayReset = 48; // OLED reset pin
#endif
  boardDefaultPins.outputs.pinTachOut = 49; //Tacho output pin
  boardDefaultPins.outputs.pinIdle1 = 5; //Single wire idle control
  boardDefaultPins.outputs.pinIdle2 = 53; //2 wire idle control
  boardDefaultPins.outputs.pinBoost = 7; //Boost control
  boardDefaultPins.outputs.pinVVT_1 = 6; //Default VVT output
  boardDefaultPins.outputs.pinVVT_2 = 48; //Default VVT2 output
  boardDefaultPins.outputs.pinFuelPump = 4; //Fuel pump output
  boardDefaultPins.outputs.pinStepperDir = 16; //Direction pin  for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = 17; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinStepperEnable = 26; //Enable pin for DRV8825
  boardDefaultPins.outputs.pinFan = A13; //Pin for the fan output
  boardDefaultPins.inputs.pinLaunch = 51; //Can be overwritten below
  boardDefaultPins.inputs.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
  boardDefaultPins.outputs.pinResetControl = 50; //Reset control output
  boardDefaultPins.inputs.sensors.pinBaro = A5;
  boardDefaultPins.inputs.pinVSS = 20;

  #if defined(CORE_TEENSY35)
    boardDefaultPins.inputs.pinTrigger = 23;
    boardDefaultPins.outputs.pinStepperDir = 33;
    boardDefaultPins.outputs.pinStepperStep = 34;
    boardDefaultPins.outputs.pinTachOut = 28;
    boardDefaultPins.outputs.pinFan = 27;
    boardDefaultPins.inputs.sensors.pinO2 = A22;
  #endif

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingsMx5Nb2Shield(void) {
  pin_mapping_t boardDefaultPins = { };

  // 2001-05 MX5 PNP shield
  // (this is the NB2 generation MX-5/Miata)

  static const uint8_t boardFuelPins[] PROGMEM = { 44, 46, 47, 45, 14 };
#if defined(CORE_TEENSY35)
  //This is NOT correct. It has not yet been tested with this board
  static const uint8_t boardCoilPins[] PROGMEM = { 33, 24, 51, 52, 34 };
#else
  static const uint8_t boardCoilPins[] PROGMEM = { 42, 43, 32, 33, 34 };
    #endif
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 19; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 18; //The Cam Sensor pin
  boardDefaultPins.inputs.pinTrigger3 = 2; //The Cam sensor 2 pin
  boardDefaultPins.inputs.sensors.pinTPS = A2;//TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A5; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A0; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A1; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A3; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A4; //Battery reference voltage pin
#if defined(ENABLE_DISPLAY_RESET)
  boardDefaultPins.inputs.pinDisplayReset = 48; // OLED reset pin
#endif
  boardDefaultPins.outputs.pinTachOut = 23; //Tacho output pin  (Goes to ULN2803)
  boardDefaultPins.outputs.pinIdle1 = 5; //Single wire idle control
  boardDefaultPins.outputs.pinBoost = 4;
  boardDefaultPins.outputs.pinVVT_1 = 11; //Default VVT output
  boardDefaultPins.outputs.pinVVT_2 = 48; //Default VVT2 output
  boardDefaultPins.outputs.pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
  boardDefaultPins.outputs.pinFuelPump = 40; //Fuel pump output
  boardDefaultPins.outputs.pinStepperDir = 16; //Direction pin  for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = 17; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinStepperEnable = 24;
  boardDefaultPins.outputs.pinFan = 41; //Pin for the fan output
  boardDefaultPins.inputs.pinLaunch = 12; //Can be overwritten below
  boardDefaultPins.inputs.pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
  boardDefaultPins.outputs.pinResetControl = 39; //Reset control output

  //This is NOT correct. It has not yet been tested with this board
#if defined(CORE_TEENSY35)
    boardDefaultPins.inputs.pinTrigger = 23;
    boardDefaultPins.inputs.pinTrigger2 = 36;
    boardDefaultPins.outputs.pinStepperDir = 34;
    boardDefaultPins.outputs.pinStepperStep = 35;
    boardDefaultPins.outputs.pinFuelPump = 26; //Requires PVT4 adapter or above
    boardDefaultPins.outputs.pinFan = 50; //Won't work (No mapping for pin 35)
    boardDefaultPins.outputs.pinTachOut = 28; //Done
#endif

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingsMx5Na18Shield(void) {
  pin_mapping_t boardDefaultPins = { };
  // 1996-97 MX5 PNP shield
  // (this is the NA 1.8 generation MX-5/Miata)

  static const uint8_t boardFuelPins[] PROGMEM = { 11, 10, 9, 8, 14 };
#if defined(CORE_TEENSY35)
  //This is NOT correct. It has not yet been tested with this board
  static const uint8_t boardCoilPins[] PROGMEM = { 33, 24, 51, 52, 34 };
#else
  static const uint8_t boardCoilPins[] PROGMEM = { 39, 41, 32, 33, 34 };
#endif
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 19; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 18; //The Cam Sensor pin
  boardDefaultPins.inputs.sensors.pinTPS = A2;//TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A5; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A0; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A1; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A3; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A4; //Battery reference voltage pin
#if defined(ENABLE_DISPLAY_RESET)
  boardDefaultPins.inputs.pinDisplayReset = 48; // OLED reset pin
#endif
  boardDefaultPins.outputs.pinTachOut = A9; //Tacho output pin  (Goes to ULN2803)
  boardDefaultPins.outputs.pinIdle1 = 2; //Single wire idle control
  boardDefaultPins.outputs.pinBoost = 4;
  boardDefaultPins.outputs.pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
  boardDefaultPins.outputs.pinFuelPump = 49; //Fuel pump output
  boardDefaultPins.outputs.pinStepperDir = 16; //Direction pin  for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = 17; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinStepperEnable = 24;
  boardDefaultPins.outputs.pinFan = 35; //Pin for the fan output
  boardDefaultPins.inputs.pinLaunch = 37; //Can be overwritten below
  boardDefaultPins.inputs.pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
  boardDefaultPins.outputs.pinResetControl = 44; //Reset control output

  //This is NOT correct. It has not yet been tested with this board
#if defined(CORE_TEENSY35)
  boardDefaultPins.inputs.pinTrigger = 23;
  boardDefaultPins.inputs.pinTrigger2 = 36;
  boardDefaultPins.outputs.pinStepperDir = 34;
  boardDefaultPins.outputs.pinStepperStep = 35;
  boardDefaultPins.outputs.pinFuelPump = 26; //Requires PVT4 adapter or above
  boardDefaultPins.outputs.pinFan = 50; //Won't work (No mapping for pin 35)
  boardDefaultPins.outputs.pinTachOut = 28; //Done
#endif

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingsMx5Na16Shield(void) {
  pin_mapping_t boardDefaultPins = { };
  // 89-96 MX5 PNP shield
  // (this is the NA 1.6 generation MX-5/Miata)

  static const uint8_t boardFuelPins[] PROGMEM = { 11, 10, 9, 8, 14 };
#if defined(CORE_TEENSY35)
  //This is NOT correct. It has not yet been tested with this board
  static const uint8_t boardCoilPins[] PROGMEM = { 33, 24, 51, 52, 34 };
#else
  static const uint8_t boardCoilPins[] PROGMEM = { 39, 41, 32, 33, 34 };
#endif
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 19; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 18; //The Cam Sensor pin
  boardDefaultPins.inputs.sensors.pinTPS = A2;//TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A5; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A0; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A1; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A3; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A4; //Battery reference voltage pin
#if defined(ENABLE_DISPLAY_RESET)
  boardDefaultPins.inputs.pinDisplayReset = 48; // OLED reset pin
#endif
  boardDefaultPins.outputs.pinTachOut = 49; //Tacho output pin  (Goes to ULN2803)
  boardDefaultPins.outputs.pinIdle1 = 2; //Single wire idle control
  boardDefaultPins.outputs.pinBoost = 4;
  boardDefaultPins.outputs.pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
  boardDefaultPins.outputs.pinFuelPump = 37; //Fuel pump output
  //Note that there is no stepper driver output on the PNP boards. These pins are unconnected and remain here just to prevent issues with random pin numbers occurring
  boardDefaultPins.outputs.pinStepperEnable = 15; //Enable pin for the DRV8825
  boardDefaultPins.outputs.pinStepperDir = 16; //Direction pin  for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = 17; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinFan = 35; //Pin for the fan output
  boardDefaultPins.inputs.pinLaunch = 12; //Can be overwritten below
  boardDefaultPins.inputs.pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
  boardDefaultPins.outputs.pinResetControl = 44; //Reset control output
  boardDefaultPins.inputs.pinVSS = 20;
  boardDefaultPins.inputs.pinIdleUp = 48;
  boardDefaultPins.inputs.pinCTPS = 47;
#if defined(CORE_TEENSY35)
  boardDefaultPins.inputs.pinTrigger = 23;
  boardDefaultPins.inputs.pinTrigger2 = 36;
  boardDefaultPins.outputs.pinStepperDir = 34;
  boardDefaultPins.outputs.pinStepperStep = 35;
  boardDefaultPins.outputs.pinFuelPump = 26; //Requires PVT4 adapter or above
  boardDefaultPins.outputs.pinFan = 50; //Won't work (No mapping for pin 35)
  boardDefaultPins.outputs.pinTachOut = 28; //Done
#endif

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingsTurtanasPcb(void) {
  pin_mapping_t boardDefaultPins = { };
  //Pin mappings for user turtanas PCB

  static const uint8_t boardFuelPins[] PROGMEM = { 4, 5, 6, 7, 8, 9, 10, 11 };
  static const uint8_t boardCoilPins[] PROGMEM = { 24, 28, 36, 40, 34 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 18; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 19; //The Cam Sensor pin
  boardDefaultPins.inputs.sensors.pinTPS = A2;//TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A3; //MAP sensor pin
#if defined(USE_MAP2)
  boardDefaultPins.inputs.sensors.pinMAP2 = A8; //MAP2 sensor pin
#endif
  boardDefaultPins.inputs.sensors.pinIAT = A0; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A1; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A4; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A7; //Battery reference voltage pin
#if defined(ENABLE_DISPLAY_RESET)
  boardDefaultPins.inputs.pinDisplayReset = 48; // OLED reset pin
#endif
#if defined(ENABLE_SPARE_TEMP)
  boardDefaultPins.inputs.pinSpareTemp1 = A6;
  boardDefaultPins.inputs.pinSpareTemp2 = A5;
#endif
  boardDefaultPins.outputs.pinTachOut = 41; //Tacho output pin transistor is missing 2n2222 for this and 1k for 12v
  boardDefaultPins.outputs.pinFuelPump = 42; //Fuel pump output 2n2222
  boardDefaultPins.outputs.pinFan = 47; //Pin for the fan output
  boardDefaultPins.outputs.pinTachOut = 49; //Tacho output pin
  boardDefaultPins.inputs.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
  boardDefaultPins.outputs.pinResetControl = 26; //Reset control  

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingsDazV6Shield(void) {
  pin_mapping_t boardDefaultPins = { };
  //Pin mappings as per the dazv6 shield

  static const uint8_t boardFuelPins[] PROGMEM = { 8, 9, 10, 11, 12 };
  static const uint8_t boardCoilPins[] PROGMEM = { 40, 38, 50, 52, 34 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 19; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 18; //The Cam Sensor pin
  boardDefaultPins.inputs.pinTrigger3 = 17; // cam sensor 2 pin, pin17 isn't external trigger enabled in arduino mega??
  boardDefaultPins.inputs.sensors.pinTPS = A2;//TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A3; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A0; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A1; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A8; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinO2_2 = A9; //O2 sensor pin (second sensor)
  boardDefaultPins.inputs.sensors.pinBat = A4; //Battery reference voltage pin
#if defined(ENABLE_DISPLAY_RESET)
  boardDefaultPins.inputs.pinDisplayReset = 48; // OLED reset pin
#endif
  boardDefaultPins.outputs.pinTachOut = 49; //Tacho output pin
  boardDefaultPins.outputs.pinIdle1 = 5; //Single wire idle control
  boardDefaultPins.outputs.pinFuelPump = 45; //Fuel pump output
  boardDefaultPins.outputs.pinStepperDir = 20; //Direction pin  for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = 21; //Step pin for DRV8825 driver
#if defined(ENABLE_SPARE_HIGH_OUTPUT)
  boardDefaultPins.outputs.pinSpareHOut1 = 4; // high current output spare1
  boardDefaultPins.outputs.pinSpareHOut2 = 6; // high current output spare2
#endif
  boardDefaultPins.outputs.pinBoost = 7;
#if defined(ENABLE_SPARE_LOW_OUTPUT)
  boardDefaultPins.outputs.pinSpareLOut1 = 43; //low current output spare1
  boardDefaultPins.outputs.pinSpareLOut2 = 47;
  boardDefaultPins.outputs.pinSpareLOut3 = 49;
  boardDefaultPins.outputs.pinSpareLOut4 = 51;
  boardDefaultPins.outputs.pinSpareLOut5 = 53;
#endif
  boardDefaultPins.outputs.pinFan = 47; //Pin for the fan output

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingsNO2CShield(void) {
  pin_mapping_t boardDefaultPins = { };
  //Pin mappings as per the NO2C shield

  static const uint8_t boardFuelPins[] PROGMEM = { 8, 9, 11, 12, 13 };
  static const uint8_t boardCoilPins[] PROGMEM = { 23, 22, 2, 3, 46 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 19; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 18; //The Cam Sensor pin
  boardDefaultPins.inputs.pinTrigger3 = 21; //The Cam sensor 2 pin
  boardDefaultPins.inputs.sensors.pinTPS = A3; //TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A0; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A5; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A4; //CLT sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A2; //O2 sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A1; //Battery reference voltage pin
  boardDefaultPins.inputs.sensors.pinBaro = A6; //Baro sensor pin - ONLY WITH DB
#if defined(ENABLE_SPARE_TEMP)
  boardDefaultPins.inputs.pinSpareTemp1 = A7; //spare Analog input 1 - ONLY WITH DB
#endif
#if defined(ENABLE_DISPLAY_RESET)
  boardDefaultPins.inputs.pinDisplayReset = 48; // OLED reset pin
#endif
  boardDefaultPins.outputs.pinTachOut = 38; //Tacho output pin
  boardDefaultPins.outputs.pinIdle1 = 5; //Single wire idle control
  boardDefaultPins.outputs.pinIdle2 = 47; //2 wire idle control - NOT USED
  boardDefaultPins.outputs.pinBoost = 7; //Boost control
  boardDefaultPins.outputs.pinVVT_1 = 6; //Default VVT output
  boardDefaultPins.outputs.pinVVT_2 = 48; //Default VVT2 output
  boardDefaultPins.outputs.pinFuelPump = 4; //Fuel pump output
  boardDefaultPins.outputs.pinStepperDir = 25; //Direction pin for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = 24; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinStepperEnable = 27; //Enable pin for DRV8825 driver
  boardDefaultPins.inputs.pinLaunch = 10; //Can be overwritten below
  boardDefaultPins.inputs.pinFlex = 20; // Flex sensor (Must be external interrupt enabled) - ONLY WITH DB
  boardDefaultPins.outputs.pinFan = 30; //Pin for the fan output - ONLY WITH DB
#if defined(ENABLE_SPARE_LOW_OUTPUT)
  boardDefaultPins.outputs.pinSpareLOut1 = 32; //low current output spare1 - ONLY WITH DB
  boardDefaultPins.outputs.pinSpareLOut2 = 34; //low current output spare2 - ONLY WITH DB
  boardDefaultPins.outputs.pinSpareLOut3 = 36; //low current output spare3 - ONLY WITH DB
#endif
  boardDefaultPins.outputs.pinResetControl = 26; //Reset control output

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingsUA4CShield(void) {
  pin_mapping_t boardDefaultPins = { };
  //Pin mappings as per the UA4C shield
  static const uint8_t boardFuelPins[] PROGMEM = { 8, 7, 6, 5, 45 };
  static const uint8_t boardCoilPins[] PROGMEM = { 35, 36, 33, 34, 44 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 19; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 18; //The Cam Sensor pin
  boardDefaultPins.inputs.pinTrigger3 = 3; //The Cam sensor 2 pin
  boardDefaultPins.inputs.pinFlex = 20; // Flex sensor
  boardDefaultPins.inputs.sensors.pinTPS = A3; //TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A0; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinBaro = A7; //Baro sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A5; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A4; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A1; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinO2_2 = A9; //O2 sensor pin (second sensor)
  boardDefaultPins.inputs.sensors.pinBat = A2; //Battery reference voltage pin
#if defined(ENABLE_SPARE_TEMP)
  boardDefaultPins.inputs.pinSpareTemp1 = A8; //spare Analog input 1
#endif
  boardDefaultPins.inputs.pinLaunch = 37; //Can be overwritten below
#if defined(ENABLE_DISPLAY_RESET)
  boardDefaultPins.inputs.pinDisplayReset = 48; // OLED reset pin
#endif
  boardDefaultPins.outputs.pinTachOut = 22; //Tacho output pin
  boardDefaultPins.outputs.pinIdle1 = 9; //Single wire idle control
  boardDefaultPins.outputs.pinIdle2 = 10; //2 wire idle control
  boardDefaultPins.outputs.pinFuelPump = 23; //Fuel pump output
  boardDefaultPins.outputs.pinVVT_1 = 11; //Default VVT output
  boardDefaultPins.outputs.pinVVT_2 = 48; //Default VVT2 output
  boardDefaultPins.outputs.pinStepperDir = 32; //Direction pin  for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = 31; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinStepperEnable = 30; //Enable pin for DRV8825 driver
  boardDefaultPins.outputs.pinBoost = 12; //Boost control
#if defined(ENABLE_SPARE_LOW_OUTPUT)
  boardDefaultPins.outputs.pinSpareLOut1 = 26; //low current output spare1
  boardDefaultPins.outputs.pinSpareLOut2 = 27; //low current output spare2
  boardDefaultPins.outputs.pinSpareLOut3 = 28; //low current output spare3
  boardDefaultPins.outputs.pinSpareLOut4 = 29; //low current output spare4
#endif
  boardDefaultPins.outputs.pinFan = 24; //Pin for the fan output
  boardDefaultPins.outputs.pinResetControl = 46; //Reset control output PLACEHOLDER value for now

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingsBlitzboxBL49sp(void) {
  pin_mapping_t boardDefaultPins = { };
  //Pin mappings for all BlitzboxBL49sp variants
  static const uint8_t boardFuelPins[] PROGMEM = { 6, 7, 8, 9 };
  static const uint8_t boardCoilPins[] PROGMEM = { 24, 25, 23, 22 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 19; //The CRANK Sensor pin
  boardDefaultPins.inputs.pinTrigger2 = 18; //The Cam Sensor pin
  boardDefaultPins.inputs.pinFlex = 20; // Flex sensor PLACEHOLDER value for now
  boardDefaultPins.inputs.sensors.pinTPS = A0; //TPS input pin
#if defined(ENABLE_SPARE_TEMP)
  boardDefaultPins.inputs.pinSpareTemp1 = A1; //LMM sensor pin
#endif
  boardDefaultPins.inputs.sensors.pinO2 = A2; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A3; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A4; //CLT sensor pin
  boardDefaultPins.inputs.sensors.pinMAP = A7; //internal MAP sensor
  boardDefaultPins.inputs.sensors.pinBat = A6; //Battery reference voltage pin
  boardDefaultPins.inputs.sensors.pinBaro = A5; //external MAP/Baro sensor pin
  boardDefaultPins.inputs.sensors.pinO2_2 = A9; //O2 sensor pin (second sensor) PLACEHOLDER value for now
  boardDefaultPins.inputs.pinLaunch = 2; //Can be overwritten below
  boardDefaultPins.outputs.pinTachOut = 10; //Tacho output pin
  boardDefaultPins.outputs.pinIdle1 = 11; //Single wire idle control
  boardDefaultPins.outputs.pinIdle2 = 14; //2 wire idle control PLACEHOLDER value for now
  boardDefaultPins.outputs.pinFuelPump = 3; //Fuel pump output
  boardDefaultPins.outputs.pinVVT_1 = 15; //Default VVT output PLACEHOLDER value for now
  boardDefaultPins.outputs.pinBoost = 13; //Boost control
#if defined(ENABLE_SPARE_LOW_OUTPUT)  
  boardDefaultPins.outputs.pinSpareLOut1 = 49; //enable Wideband Lambda Heater
  boardDefaultPins.outputs.pinSpareLOut2 = 16; //low current output spare2 PLACEHOLDER value for now
  boardDefaultPins.outputs.pinSpareLOut3 = 17; //low current output spare3 PLACEHOLDER value for now
  boardDefaultPins.outputs.pinSpareLOut4 = 21; //low current output spare4 PLACEHOLDER value for now
#endif
  boardDefaultPins.outputs.pinFan = 12; //Pin for the fan output
  boardDefaultPins.outputs.pinResetControl = 46; //Reset control output PLACEHOLDER value for now

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingsDiyEfiCore4Shield(void) {
  pin_mapping_t boardDefaultPins = { };
  //Pin mappings for the DIY-EFI CORE4 Module. This is an AVR only module
#if defined(CORE_AVR)
  static const uint8_t boardFuelPins[] PROGMEM = { 10, 11, 12, 9 };
  static const uint8_t boardCoilPins[] PROGMEM = { 39, 29, 28, 27, 33, 34 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 19; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 18; //The Cam Sensor pin
  boardDefaultPins.inputs.pinTrigger3 = 21;// The Cam sensor 2 pin
  boardDefaultPins.inputs.pinFlex = 20; // Flex sensor
  boardDefaultPins.inputs.sensors.pinTPS = A3; //TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A2; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinBaro = A15; //Baro sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A11; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A4; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A12; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinO2_2 = A5; //O2 sensor pin (second sensor)
  boardDefaultPins.inputs.sensors.pinBat = A1; //Battery reference voltage pin
#if defined(ENABLE_SPARE_TEMP)
  boardDefaultPins.inputs.pinSpareTemp1 = A14; //spare Analog input 1
#endif
  boardDefaultPins.inputs.pinLaunch = 24; //Can be overwritten below
#if defined(ENABLE_DISPLAY_RESET)
  boardDefaultPins.inputs.pinDisplayReset = 48; // OLED reset pin
#endif
  boardDefaultPins.outputs.pinTachOut = 38; //Tacho output pin
  boardDefaultPins.outputs.pinIdle1 = 42; //Single wire idle control
  boardDefaultPins.outputs.pinIdle2 = 43; //2 wire idle control
  boardDefaultPins.outputs.pinFuelPump = 41; //Fuel pump output
  boardDefaultPins.outputs.pinVVT_1 = 44; //Default VVT output
  boardDefaultPins.outputs.pinVVT_2 = 48; //Default VVT2 output
  boardDefaultPins.outputs.pinStepperDir = 32; //Direction pin  for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = 31; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinStepperEnable = 30; //Enable pin for DRV8825 driver
  boardDefaultPins.outputs.pinBoost = 45; //Boost control
#if defined(ENABLE_SPARE_LOW_OUTPUT)
  boardDefaultPins.outputs.pinSpareLOut1 = 37; //low current output spare1
  boardDefaultPins.outputs.pinSpareLOut2 = 36; //low current output spare2
  boardDefaultPins.outputs.pinSpareLOut3 = 35; //low current output spare3
#endif
  boardDefaultPins.outputs.pinFan = 40; //Pin for the fan output
  boardDefaultPins.outputs.pinResetControl = 46; //Reset control output PLACEHOLDER value for now
#endif // CORE_AVR

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingsPlazomatV0_1Shield(void) {
  pin_mapping_t boardDefaultPins = { };
#if defined(CORE_AVR)
  //Pin mappings as per the Plazomat In/Out shields Rev 0.1

  static const uint8_t boardFuelPins[] PROGMEM = { 8, 9, 10, 11, 12 };
  static const uint8_t boardCoilPins[] PROGMEM = { 28, 24, 40, 36, 34 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

#if defined(ENABLE_SPARE_OUTPUT)
  boardDefaultPins.outputs.pinSpareOut1 = 4; //Spare LSD Output 1(PWM)
  boardDefaultPins.outputs.pinSpareOut2 = 5; //Spare LSD Output 2(PWM)
  boardDefaultPins.outputs.pinSpareOut3 = 6; //Spare LSD Output 3(PWM)
  boardDefaultPins.outputs.pinSpareOut4 = 7; //Spare LSD Output 4(PWM)
  boardDefaultPins.outputs.pinSpareOut5 = 50; //Spare LSD Output 5(digital)
  boardDefaultPins.outputs.pinSpareOut6 = 52; //Spare LSD Output 6(digital)
#endif
  boardDefaultPins.inputs.pinTrigger = 20; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 21; //The Cam Sensor pin
#if defined(ENABLE_SPARE_TEMP)
  boardDefaultPins.inputs.pinSpareTemp2 = A15; //spare Analog input 2
  boardDefaultPins.inputs.pinSpareTemp1 = A14; //spare Analog input 1
#endif
  boardDefaultPins.inputs.sensors.pinO2 = A8; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A4; //Battery reference voltage pin
  boardDefaultPins.inputs.sensors.pinMAP = A3; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinTPS = A2;//TPS input pin
  boardDefaultPins.inputs.sensors.pinCLT = A1; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A0; //IAT sensor pin
  boardDefaultPins.outputs.pinFan = 47; //Pin for the fan output
  boardDefaultPins.outputs.pinFuelPump = 4; //Fuel pump output
  boardDefaultPins.outputs.pinTachOut = 49; //Tacho output pin
  boardDefaultPins.outputs.pinResetControl = 26; //Reset control output
#endif // CORE_AVR

  return boardDefaultPins;
}

#endif // SMALL_FLASH_MODE

static pin_mapping_t getPinMappingsV4_0Shield(void) {
  pin_mapping_t boardDefaultPins = { };
  //Pin mappings as per the v0.4 shield
#if defined(CORE_TEENSY35)
  static const uint8_t boardFuelPins[] PROGMEM = { 8, 9, 10, 11, 12, 50, 51 };
  static const uint8_t boardCoilPins[] PROGMEM = { 31, 32, 30, 29, 34 };
#elif defined(CORE_TEENSY41)
  static const uint8_t boardFuelPins[] PROGMEM = { 8, 9, 10, 11, 12, 50 };
  static const uint8_t boardCoilPins[] PROGMEM = { 31, 32, 30, 29, 34 };
#elif defined(STM32F407xx)
  //Black F407VE wiki.stm32duino.com/index.php?title=STM32F407
  static const uint8_t boardFuelPins[] PROGMEM = { PD7, PB9, PA8, PB10, PD9 };
  static const uint8_t boardCoilPins[] PROGMEM = { PD12, PD13, PD14, PD15, PE11, PE12 };
#elif defined(CORE_STM32)
  //https://github.com/stm32duino/Arduino_Core_STM32/blob/master/variants/Generic_F411Cx/variant.h#L28
  static const uint8_t boardFuelPins[] PROGMEM = { PB7, PB6, PB5, PB4 };
  static const uint8_t boardCoilPins[] PROGMEM = { PB9, PB8, PB3, PA15 };
#else
  static const uint8_t boardFuelPins[] PROGMEM = { 8, 9, 10, 11, 12, 50 };
  static const uint8_t boardCoilPins[] PROGMEM = { 40, 38, 52, 50, 34 };
#endif
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 19; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 18; //The Cam Sensor pin
  boardDefaultPins.inputs.pinTrigger3 = 3; //The Cam sensor 2 pin
  boardDefaultPins.inputs.sensors.pinTPS = A2;//TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A3; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A0; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A1; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A8; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A4; //Battery reference voltage pin
#if defined(ENABLE_DISPLAY_RESET)
  boardDefaultPins.inputs.pinDisplayReset = 48; // OLED reset pin
#endif
  boardDefaultPins.outputs.pinTachOut = 49; //Tacho output pin  (Goes to ULN2803)
  boardDefaultPins.outputs.pinIdle1 = 5; //Single wire idle control
  boardDefaultPins.outputs.pinIdle2 = 6; //2 wire idle control
  boardDefaultPins.outputs.pinBoost = 7; //Boost control
  boardDefaultPins.outputs.pinVVT_1 = 4; //Default VVT output
  boardDefaultPins.outputs.pinVVT_2 = 48; //Default VVT2 output
  boardDefaultPins.outputs.pinFuelPump = 45; //Fuel pump output  (Goes to ULN2803)
  boardDefaultPins.outputs.pinStepperDir = 16; //Direction pin  for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = 17; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinStepperEnable = 24; //Enable pin for DRV8825
  boardDefaultPins.outputs.pinFan = 47; //Pin for the fan output (Goes to ULN2803)
  boardDefaultPins.inputs.pinLaunch = 51; //Can be overwritten below
  boardDefaultPins.inputs.pinFlex = 2; // Flex sensor (Must be external interrupt enabled)
  boardDefaultPins.outputs.pinResetControl = 43; //Reset control output
  boardDefaultPins.inputs.sensors.pinBaro = A5;
  boardDefaultPins.inputs.pinVSS = 20;
  boardDefaultPins.inputs.pinWMIEmpty = 46;
  boardDefaultPins.outputs.pinWMIIndicator = 44;
  boardDefaultPins.outputs.pinWMIEnabled = 42;

  #if defined(CORE_TEENSY35)
  boardDefaultPins.inputs.pinTrigger = 23;
  boardDefaultPins.inputs.pinTrigger2 = 36;
  boardDefaultPins.outputs.pinStepperDir = 34;
  boardDefaultPins.outputs.pinStepperStep = 35;
  boardDefaultPins.outputs.pinTachOut = 28;
  boardDefaultPins.outputs.pinFan = 27;
  boardDefaultPins.inputs.sensors.pinO2 = A22;
  //Make sure the CAN pins aren't overwritten
  boardDefaultPins.inputs.pinTrigger3 = 54;
  boardDefaultPins.outputs.pinVVT_1 = 55;
#elif defined(CORE_TEENSY41)
  //These are only to prevent lockups or weird behaviour on T4.1 when this board is used as the default
  boardDefaultPins.inputs.sensors.pinBaro = A4; 
  boardDefaultPins.inputs.sensors.pinMAP = A5;
  boardDefaultPins.inputs.sensors.pinTPS = A3; //TPS input pin
  boardDefaultPins.inputs.sensors.pinIAT = A0; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A1; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A2; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A15; //Battery reference voltage pin. Needs Alpha4+
  boardDefaultPins.inputs.pinLaunch = 34; //Can be overwritten below
  boardDefaultPins.inputs.pinVSS = 35;
#if defined(ENABLE_SPARE_TEMP)
  boardDefaultPins.inputs.pinSpareTemp2 = A16; //WRONG! Needs updating!!
  boardDefaultPins.inputs.pinSpareTemp2 = A17; //WRONG! Needs updating!!
#endif

  boardDefaultPins.inputs.pinTrigger = 20; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 21; //The Cam Sensor pin
  boardDefaultPins.inputs.pinTrigger3 = 24;

  boardDefaultPins.outputs.pinStepperDir = 34;
  boardDefaultPins.outputs.pinStepperStep = 35;

  boardDefaultPins.outputs.pinTachOut = 28;
  boardDefaultPins.outputs.pinFan = 27;
  boardDefaultPins.outputs.pinFuelPump = 33;
  boardDefaultPins.inputs.pinWMIEmpty = 34;
  boardDefaultPins.outputs.pinWMIIndicator = 35;
  boardDefaultPins.outputs.pinWMIEnabled = 36;
#elif defined(STM32F407xx)
//Pin definitions for experimental board Tjeerd 
  //Black F407VE wiki.stm32duino.com/index.php?title=STM32F407

  //******************************************
  //******** PORTA CONNECTIONS *************** 
  //******************************************
  /* = PA0 */ //Wakeup ADC123
  // = PA1;
  // = PA2;
  // = PA3;
  // = PA4;
  /* = PA5; */ //ADC12
  /* = PA6; */ //ADC12 LED_BUILTIN_1
  boardDefaultPins.outputs.pinFuelPump = PA7; //ADC12 LED_BUILTIN_2
  /* = PA9 */ //TXD1
  /* = PA10 */ //RXD1
  /* = PA11 */ //(DO NOT USE FOR SPEEDUINO) USB
  /* = PA12 */ //(DO NOT USE FOR SPEEDUINO) USB 
  /* = PA13 */ //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  /* = PA14 */ //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  /* = PA15 */ //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK

  //******************************************
  //******** PORTB CONNECTIONS *************** 
  //******************************************
  /* = PB0; */ //(DO NOT USE FOR SPEEDUINO) ADC123 - SPI FLASH CHIP CS pin
  boardDefaultPins.inputs.sensors.pinBaro = PB1; //ADC12
  /* = PB2; */ //(DO NOT USE FOR SPEEDUINO) BOOT1 
  /* = PB3; */ //(DO NOT USE FOR SPEEDUINO) SPI1_SCK FLASH CHIP
  /* = PB4; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MISO FLASH CHIP
  /* = PB5; */ //(DO NOT USE FOR SPEEDUINO) SPI1_MOSI FLASH CHIP
  /* = PB6; */ //NRF_CE
  /* = PB7; */ //NRF_CS
  /* = PB8; */ //NRF_IRQ
  /* = PB9; */ //
  boardDefaultPins.outputs.pinIdle1 = PB11; //RXD3
  boardDefaultPins.outputs.pinIdle2 = PB12; //
  boardDefaultPins.outputs.pinBoost = PB12; //
  /* = PB13; */ //SPI2_SCK
  /* = PB14; */ //SPI2_MISO
  /* = PB15; */ //SPI2_MOSI

  //******************************************
  //******** PORTC CONNECTIONS *************** 
  //******************************************
  boardDefaultPins.inputs.sensors.pinMAP = PC0; //ADC123 
  boardDefaultPins.inputs.sensors.pinTPS = PC1; //ADC123
  boardDefaultPins.inputs.sensors.pinIAT = PC2; //ADC123
  boardDefaultPins.inputs.sensors.pinCLT = PC3; //ADC123
  boardDefaultPins.inputs.sensors.pinO2 = PC4;  //ADC12
  boardDefaultPins.inputs.sensors.pinBat = PC5; //ADC12
  boardDefaultPins.outputs.pinVVT_1 = PC6; //
#if defined(ENABLE_DISPLAY_RESET)
  boardDefaultPins.inputs.pinDisplayReset = PC7; // OLED reset pin
#endif
  /* = PC8; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D0
  /* = PC9; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D1
  /* = PC10; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D2
  /* = PC11; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_D3
  /* = PC12; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_SCK
  boardDefaultPins.outputs.pinTachOut = PC13; //
  /* = PC14; */ //(DO NOT USE FOR SPEEDUINO) - OSC32_IN
  /* = PC15; */ //(DO NOT USE FOR SPEEDUINO) - OSC32_OUT

  //******************************************
  //******** PORTD CONNECTIONS *************** 
  //******************************************
  /* = PD0; */ //CANRX
  /* = PD1; */ //CANTX
  /* = PD2; */ //(DO NOT USE FOR SPEEDUINO) - SDIO_CMD
  boardDefaultPins.outputs.pinVVT_2 = PD3; //
  boardDefaultPins.inputs.pinFlex = PD4;
  /* = PD5;*/ //TXD2
  /* = PD6; */ //RXD2
  /* = PD8; */ //
  /* = PD10; */ //
  /* = PD11; */ //

  //******************************************
  //******** PORTE CONNECTIONS *************** 
  //******************************************
  boardDefaultPins.inputs.pinTrigger = PE0; //
  boardDefaultPins.inputs.pinTrigger2 = PE1; //
  boardDefaultPins.outputs.pinStepperEnable = PE2; //
  /* = PE3; */ //ONBOARD KEY1
  /* = PE4; */ //ONBOARD KEY2
  boardDefaultPins.outputs.pinStepperStep = PE5; //
  boardDefaultPins.outputs.pinFan = PE6; //
  boardDefaultPins.outputs.pinStepperDir = PE7; //
  /* = PE8; */ //
  /* = PE9; */ //
  /* = PE10; */ //
  /* = PE13; */ //
  /* = PE14; */ //
  /* = PE15; */ //
#elif defined(CORE_STM32)
  //https://github.com/stm32duino/Arduino_Core_STM32/blob/master/variants/Generic_F411Cx/variant.h#L28
  //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
  //pins PB12, PB13, PB14 and PB15 are used to SPI FLASH
  //PB2 can't be used as input because it's the BOOT pin
  boardDefaultPins.inputs.sensors.pinTPS = A2;//TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A3; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A0; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A1; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A8; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A4; //Battery reference voltage pin
  boardDefaultPins.inputs.sensors.pinBaro = boardDefaultPins.inputs.sensors.pinMAP;
  boardDefaultPins.outputs.pinTachOut = PB1; //Tacho output pin  (Goes to ULN2803)
  boardDefaultPins.outputs.pinIdle1 = PB2; //Single wire idle control
  boardDefaultPins.outputs.pinIdle2 = PB10; //2 wire idle control
  boardDefaultPins.outputs.pinBoost = PA6; //Boost control
  boardDefaultPins.outputs.pinStepperDir = PB10; //Direction pin  for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = PB2; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinFuelPump = PA8; //Fuel pump output
  boardDefaultPins.outputs.pinFan = PA5; //Pin for the fan output (Goes to ULN2803)
  //external interrupt enabled pins
  boardDefaultPins.inputs.pinFlex = PC14; // Flex sensor (Must be external interrupt enabled)
  boardDefaultPins.inputs.pinTrigger = PC13; //The CAS pin also led pin so bad idea
  boardDefaultPins.inputs.pinTrigger2 = PC15; //The Cam Sensor pin
#endif

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingBmwPnP(void) {
  pin_mapping_t boardDefaultPins = { };
#if defined(CORE_AVR)
      //This is the regular MEGA2560 pin mapping
  static const uint8_t boardFuelPins[] PROGMEM = { 8, 9, 10, 11, 12, 50, 39, 42 };
  static const uint8_t boardCoilPins[] PROGMEM = { 40, 38, 52, 48, 36, 34, 46, 53 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 19; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 18; //The Cam Sensor pin
  boardDefaultPins.inputs.pinTrigger3 = 20; //The Cam sensor 2 pin
  boardDefaultPins.inputs.sensors.pinTPS = A2;//TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A3; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinEMAP = A15; //EMAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A0; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A1; //CLT sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A8; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A4; //Battery reference voltage pin
  boardDefaultPins.inputs.sensors.pinBaro = A5; //Baro sensor pin
#if defined(ENABLE_DISPLAY_RESET)
  boardDefaultPins.inputs.pinDisplayReset = 41; // OLED reset pin
#endif
  boardDefaultPins.outputs.pinTachOut = 49; //Tacho output pin  (Goes to ULN2003)
  boardDefaultPins.outputs.pinIdle1 = 5; //ICV pin1
  boardDefaultPins.outputs.pinIdle2 = 6; //ICV pin3
  boardDefaultPins.outputs.pinBoost = 7; //Boost control
  boardDefaultPins.outputs.pinVVT_1 = 4; //VVT1 output (intake vanos)
  boardDefaultPins.outputs.pinVVT_2 = 26; //VVT2 output (exhaust vanos)
  boardDefaultPins.outputs.pinFuelPump = 45; //Fuel pump output  (Goes to ULN2003)
  boardDefaultPins.outputs.pinStepperDir = 16; //Stepper valve isn't used with these
  boardDefaultPins.outputs.pinStepperStep = 17; //Stepper valve isn't used with these
  boardDefaultPins.outputs.pinStepperEnable = 24; //Stepper valve isn't used with these
  boardDefaultPins.outputs.pinFan = 47; //Pin for the fan output (Goes to ULN2003)
  boardDefaultPins.inputs.pinLaunch = 51; //Launch control pin
  boardDefaultPins.inputs.pinFlex = 2; // Flex sensor
  boardDefaultPins.outputs.pinResetControl = 43; //Reset control output
  boardDefaultPins.inputs.pinVSS = 3; //VSS input pin
  boardDefaultPins.inputs.pinWMIEmpty = 31; //(placeholder)
  boardDefaultPins.outputs.pinWMIIndicator = 33; //(placeholder)
  boardDefaultPins.outputs.pinWMIEnabled = 35; //(placeholder)
  boardDefaultPins.inputs.pinIdleUp = 37; //(placeholder)
  boardDefaultPins.inputs.pinCTPS = A6; //(placeholder)
  #elif defined(STM32F407xx)
  static const uint8_t boardFuelPins[] PROGMEM = { PB15, PB14, PB12, PB13, PA8, PE7, PE13, PE10 };
  static const uint8_t boardCoilPins[] PROGMEM = { PE2, PE3, PC13, PE6, PE4, PE5, PE0, PB9 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = PD3; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = PD4; //The Cam Sensor pin
  boardDefaultPins.inputs.sensors.pinTPS = PA2;//TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = PA3; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinEMAP = PC5; //EMAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = PA0; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = PA1; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = PB0; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = PA4; //Battery reference voltage pin
  boardDefaultPins.inputs.sensors.pinBaro = PA5; //Baro sensor pin
#if defined(ENABLE_DISPLAY_RESET)
  boardDefaultPins.inputs.pinDisplayReset = PE12; // OLED reset pin
#endif
  boardDefaultPins.outputs.pinTachOut = PE8; //Tacho output pin  (Goes to ULN2003)
  boardDefaultPins.outputs.pinIdle1 = PD10; //ICV pin1
  boardDefaultPins.outputs.pinIdle2 = PD9; //ICV pin3
  boardDefaultPins.outputs.pinBoost = PD8; //Boost control
  boardDefaultPins.outputs.pinVVT_1 = PD11; //VVT1 output (intake vanos)
  boardDefaultPins.outputs.pinVVT_2 = PC7; //VVT2 output (exhaust vanos)
  boardDefaultPins.outputs.pinFuelPump = PE11; //Fuel pump output  (Goes to ULN2003)
  boardDefaultPins.outputs.pinStepperDir = PB10; //Stepper valve isn't used with these
  boardDefaultPins.outputs.pinStepperStep = PB11; //Stepper valve isn't used with these
  boardDefaultPins.outputs.pinStepperEnable = PA15; //Stepper valve isn't used with these
  boardDefaultPins.outputs.pinFan = PE9; //Pin for the fan output (Goes to ULN2003)
  boardDefaultPins.inputs.pinLaunch = PB8; //Launch control pin
  boardDefaultPins.inputs.pinFlex = PD7; // Flex sensor
  boardDefaultPins.outputs.pinResetControl = PB7; //Reset control output
  boardDefaultPins.inputs.pinVSS = PB6; //VSS input pin
  boardDefaultPins.inputs.pinWMIEmpty = PD15; //(placeholder)
  boardDefaultPins.outputs.pinWMIIndicator = PD13; //(placeholder)
  boardDefaultPins.outputs.pinWMIEnabled = PE15; //(placeholder)
  boardDefaultPins.inputs.pinIdleUp = PE14; //(placeholder)
  boardDefaultPins.inputs.pinCTPS = PA6; //(placeholder)
  #endif

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingsTeensyRevA(void) {
  pin_mapping_t boardDefaultPins = { };
      //Pin mappings as per the teensy rev A shield
#if defined(CORE_TREENSY35)
  static const uint8_t boardFuelPins[] PROGMEM = { 229, 30, 31, 32 };
  static const uint8_t boardCoilPins[] PROGMEM = { PE2, PE3, PC13, PE6, PE4, PE5, PE0, PB9 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 23; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 36; //The Cam Sensor pin
  boardDefaultPins.inputs.sensors.pinTPS = 16; //TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = 17; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = 14; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = 15; //CLT sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A22; //O2 sensor pin
  boardDefaultPins.inputs.sensors.pinO2_2 = A21; //O2 sensor pin (second sensor)
  boardDefaultPins.inputs.sensors.pinBat = 18; //Battery reference voltage pin
  boardDefaultPins.outputs.pinTachOut = 20; //Tacho output pin
  boardDefaultPins.outputs.pinIdle1 = 5; //Single wire idle control
  boardDefaultPins.outputs.pinBoost = 11; //Boost control
  boardDefaultPins.outputs.pinFuelPump = 38; //Fuel pump output
  boardDefaultPins.outputs.pinStepperDir = 34; //Direction pin for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = 35; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinStepperEnable = 33; //Enable pin for DRV8825 driver
  boardDefaultPins.inputs.pinLaunch = 26; //Can be overwritten below
  boardDefaultPins.outputs.pinFan = 37; //Pin for the fan output - ONLY WITH DB
#if defined(ENABLE_SPARE_HIGH_OUTPUT)
  boardDefaultPins.outputs.pinSpareHOut1 = 8; // high current output spare1
  boardDefaultPins.outputs.pinSpareHOut2 = 7; // high current output spare2
#endif
#if defined(ENABLE_SPARE_LOW_OUTPUT)
  boardDefaultPins.outputs.pinSpareLOut1 = 21; //low current output spare1
#endif
#endif

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingsTeensyRevB(void) {
  pin_mapping_t boardDefaultPins = { };
  //Pin mappings as per the teensy rev B shield
#if defined(CORE_TEENSY35)
  static const uint8_t boardFuelPins[] PROGMEM = { 2, 10, 6, 9 };
  static const uint8_t boardCoilPins[] PROGMEM = { 29, 30, 31, 32 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 23; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 36; //The Cam Sensor pin
  boardDefaultPins.inputs.sensors.pinTPS = 16; //TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = 17; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = 14; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = 15; //CLT sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A22; //O2 sensor pin
  boardDefaultPins.inputs.sensors.pinO2_2 = A21; //O2 sensor pin (second sensor)
  boardDefaultPins.inputs.sensors.pinBat = 18; //Battery reference voltage pin
  boardDefaultPins.outputs.pinTachOut = 20; //Tacho output pin
  boardDefaultPins.outputs.pinIdle1 = 5; //Single wire idle control
  boardDefaultPins.outputs.pinBoost = 11; //Boost control
  boardDefaultPins.outputs.pinFuelPump = 38; //Fuel pump output
  boardDefaultPins.outputs.pinStepperDir = 34; //Direction pin for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = 35; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinStepperEnable = 33; //Enable pin for DRV8825 driver
  boardDefaultPins.inputs.pinLaunch = 26; //Can be overwritten below
  boardDefaultPins.outputs.pinFan = 37; //Pin for the fan output - ONLY WITH DB
#if defined(ENABLE_SPARE_HIGH_OUTPUT)
  boardDefaultPins.outputs.pinSpareHOut1 = 8; // high current output spare1
  boardDefaultPins.outputs.pinSpareHOut2 = 7; // high current output spare2
#endif
#if defined(ENABLE_SPARE_LOW_OUTPUT)
  boardDefaultPins.outputs.pinSpareLOut1 = 21; //low current output spare1
#endif
#endif

  return boardDefaultPins;
}


static pin_mapping_t getPinMappingsJuiceBox(void) {
  pin_mapping_t boardDefaultPins = { };
      //Pin mappings for the Juice Box (ignition only board)
#if defined(CORE_TEENSY35)
  static const uint8_t boardFuelPins[] PROGMEM = { 2, 56, 6, 50 };
  static const uint8_t boardCoilPins[] PROGMEM = { 29, 30, 31, 32 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 37; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 38; //The Cam Sensor pin - NOT USED
  boardDefaultPins.inputs.sensors.pinTPS = A2; //TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A7; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A1; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A5; //CLT sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A0; //O2 sensor pin
  boardDefaultPins.inputs.sensors.pinO2_2 = A21; //O2 sensor pin (second sensor) - NOT USED
  boardDefaultPins.inputs.sensors.pinBat = A6; //Battery reference voltage pin
  boardDefaultPins.outputs.pinTachOut = 28; //Tacho output pin
  boardDefaultPins.outputs.pinIdle1 = 5; //Single wire idle control - NOT USED
  boardDefaultPins.outputs.pinBoost = 11; //Boost control - NOT USED
  boardDefaultPins.outputs.pinFuelPump = 24; //Fuel pump output
  boardDefaultPins.outputs.pinStepperDir = 3; //Direction pin for DRV8825 driver - NOT USED
  boardDefaultPins.outputs.pinStepperStep = 4; //Step pin for DRV8825 driver - NOT USED
  boardDefaultPins.outputs.pinStepperEnable = 6; //Enable pin for DRV8825 driver - NOT USED
  boardDefaultPins.inputs.pinLaunch = 26; //Can be overwritten below
  boardDefaultPins.outputs.pinFan = 25; //Pin for the fan output
#if defined(ENABLE_SPARE_HIGH_OUTPUT)
  boardDefaultPins.outputs.pinSpareHOut1 = 26; // high current output spare1
  boardDefaultPins.outputs.pinSpareHOut2 = 27; // high current output spare2
#endif
#if defined(ENABLE_SPARE_LOW_OUTPUT)
  boardDefaultPins.outputs.pinSpareLOut1 = 55; //low current output spare1 - NOT USED
#endif
#endif

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingsDropBear(void) {
  pin_mapping_t boardDefaultPins = { };
      #if defined(CORE_TEENSY)
      //Pin mappings for the DropBear
  // The injector pins below are not used directly as the control is via SPI through the MC33810s, 
  // however the pin numbers are set to be the SPI pins (SCLK, MOSI, MISO and CS) so that nothing 
  // else will set them as inputs
  static const uint8_t boardFuelPins[] PROGMEM = { 13, 11, 12, 10, 9, 9 /* This is deiberate*/ };
  //Dummy pins, without these pin 0 (Serial1 RX) gets overwritten
  static const uint8_t boardCoilPins[] PROGMEM = { 40, 41 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));
  
  boardDefaultPins.inputs.pinTrigger = 19; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 18; //The Cam Sensor pin
  boardDefaultPins.inputs.pinFlex = A16; // Flex sensor
  boardDefaultPins.inputs.sensors.pinMAP = A1; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinBaro = A0; //Baro sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A14; //Battery reference voltage pin
#if defined(ENABLE_SPARE_TEMP)
  boardDefaultPins.inputs.pinSpareTemp1 = A17; //spare Analog input 1
#endif
  boardDefaultPins.inputs.pinLaunch = A15; //Can be overwritten below
  boardDefaultPins.outputs.pinTachOut = 5; //Tacho output pin
  boardDefaultPins.outputs.pinIdle1 = 27; //Single wire idle control
  boardDefaultPins.outputs.pinIdle2 = 29; //2 wire idle control. Shared with Spare 1 output
  boardDefaultPins.outputs.pinFuelPump = 8; //Fuel pump output
  boardDefaultPins.outputs.pinVVT_1 = 28; //Default VVT output
  boardDefaultPins.outputs.pinStepperDir = 32; //Direction pin  for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = 31; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinStepperEnable = 30; //Enable pin for DRV8825 driver
  boardDefaultPins.outputs.pinBoost = 24; //Boost control
#if defined(ENABLE_SPARE_LOW_OUTPUT)
  boardDefaultPins.outputs.pinSpareLOut1 = 29; //low current output spare1
  boardDefaultPins.outputs.pinSpareLOut2 = 26; //low current output spare2
  boardDefaultPins.outputs.pinSpareLOut3 = 28; //low current output spare3
  boardDefaultPins.outputs.pinSpareLOut4 = 29; //low current output spare4
#endif
  boardDefaultPins.outputs.pinFan = 25; //Pin for the fan output
  boardDefaultPins.outputs.pinResetControl = 46; //Reset control output PLACEHOLDER value for now

#if defined(CORE_TEENSY35)
  boardDefaultPins.inputs.sensors.pinTPS = A22; //TPS input pin
  boardDefaultPins.inputs.sensors.pinIAT = A19; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A20; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A21; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinO2_2 = A18; //Spare 2
#endif

#if defined(CORE_TEENSY41)
  boardDefaultPins.inputs.sensors.pinTPS = A17; //TPS input pin
  boardDefaultPins.inputs.sensors.pinIAT = A14; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A15; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A16; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A3; //Battery reference voltage pin. Needs Alpha4+

  //New pins for the actual T4.1 version of the Dropbear
  boardDefaultPins.inputs.sensors.pinBaro = A4; 
  boardDefaultPins.inputs.sensors.pinMAP = A5;
  boardDefaultPins.inputs.sensors.pinTPS = A3; //TPS input pin
  boardDefaultPins.inputs.sensors.pinIAT = A0; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A1; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A2; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A15; //Battery reference voltage pin. Needs Alpha4+
  boardDefaultPins.inputs.pinLaunch = 36;
  boardDefaultPins.inputs.pinFlex = 37; // Flex sensor
#if defined(ENABLE_SPARE_TEMP)
  boardDefaultPins.inputs.pinSpareTemp1 = A16; 
  boardDefaultPins.inputs.pinSpareTemp2 = A17;
#endif

  boardDefaultPins.inputs.pinTrigger = 20; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 21; //The Cam Sensor pin

  boardDefaultPins.outputs.pinFuelPump = 5; //Fuel pump output
  boardDefaultPins.outputs.pinTachOut = 8; //Tacho output pin

  boardDefaultPins.outputs.pinResetControl = 49; //PLaceholder only. Cannot use 42-47 as these are the SD card
#endif

  boardDefaultPins.outputs.pinMC33810_1 = 10;
  boardDefaultPins.outputs.pinMC33810_2 = 9;

  //Pin alignment to the MC33810 outputs
  MC33810_BIT_INJ1 = 3;
  MC33810_BIT_INJ2 = 1;
  MC33810_BIT_INJ3 = 0;
  MC33810_BIT_INJ4 = 2;
  MC33810_BIT_IGN1 = 4;
  MC33810_BIT_IGN2 = 5;
  MC33810_BIT_IGN3 = 6;
  MC33810_BIT_IGN4 = 7;

  MC33810_BIT_INJ5 = 3;
  MC33810_BIT_INJ6 = 1;
  MC33810_BIT_INJ7 = 0;
  MC33810_BIT_INJ8 = 2;
  MC33810_BIT_IGN5 = 4;
  MC33810_BIT_IGN6 = 5;
  MC33810_BIT_IGN7 = 6;
  MC33810_BIT_IGN8 = 7;
#endif

  return boardDefaultPins;
}

static pin_mapping_t getPinMappingBearCub(void) {
  pin_mapping_t boardDefaultPins = { };
#if defined(CORE_TEENSY)
//Pin mappings for the Bear Cub (Teensy 4.1)
  static const uint8_t boardFuelPins[] PROGMEM = { 6, 7, 9, 8 };
  static const uint8_t boardCoilPins[] PROGMEM = { 2, 3, 4, 5 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.pinTrigger = 20; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = 21; //The Cam Sensor pin
  boardDefaultPins.inputs.pinFlex = 37; // Flex sensor
  boardDefaultPins.inputs.sensors.pinMAP = A5; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinBaro = A4; //Baro sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A15; //Battery reference voltage pin
  boardDefaultPins.inputs.sensors.pinTPS = A3; //TPS input pin
  boardDefaultPins.inputs.sensors.pinIAT = A0; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A1; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A2; //O2 Sensor pin
  boardDefaultPins.inputs.pinLaunch = 36;
#if defined(ENABLE_SPARE_TEMP)
  boardDefaultPins.inputs.pinSpareTemp1 = A16; //spare Analog input 1
  boardDefaultPins.inputs.pinSpareTemp2 = A17; //spare Analog input 2
#endif
  boardDefaultPins.outputs.pinTachOut = 38; //Tacho output pin
  boardDefaultPins.outputs.pinIdle1 = 27; //Single wire idle control
  boardDefaultPins.outputs.pinIdle2 = 26; //2 wire idle control. Shared with Spare 1 output
  boardDefaultPins.outputs.pinFuelPump = 10; //Fuel pump output
  boardDefaultPins.outputs.pinVVT_1 = 28; //Default VVT output
  boardDefaultPins.outputs.pinStepperDir = 32; //Direction pin  for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = 31; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinStepperEnable = 30; //Enable pin for DRV8825 driver
  boardDefaultPins.outputs.pinBoost = 24; //Boost control
#if defined(ENABLE_SPARE_LOW_OUTPUT)
  boardDefaultPins.outputs.pinSpareLOut1 = 29; //low current output spare1
  boardDefaultPins.outputs.pinSpareLOut2 = 26; //low current output spare2
  boardDefaultPins.outputs.pinSpareLOut3 = 28; //low current output spare3
  boardDefaultPins.outputs.pinSpareLOut4 = 29; //low current output spare4
#endif  
  boardDefaultPins.outputs.pinFan = 25; //Pin for the fan output
  boardDefaultPins.outputs.pinResetControl = 46; //Reset control output PLACEHOLDER value for now
#endif

  return boardDefaultPins;
}
 
static pin_mapping_t getPinMappingsSTM32(void) {
  pin_mapping_t boardDefaultPins = { };
#if defined(STM32F407xx)
  //Pin definitions for experimental board Tjeerd 
  //Black F407VE wiki.stm32duino.com/index.php?title=STM32F407
  //https://github.com/Tjeerdie/SPECTRE/tree/master/SPECTRE_V0.5

  static const uint8_t boardFuelPins[] PROGMEM = { PD12, PD13, PD14, PD15, PE9, PE11, PE14, PE13 };
  static const uint8_t boardCoilPins[] PROGMEM = { PD7, PB9, PA8, PD10, PD9 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));
  
  //******************************************
  //******** PORTA CONNECTIONS *************** 
  //******************************************
  // = PA0; //Wakeup ADC123
  // = PA1; //ADC123
  // = PA2; //ADC123
  // = PA3; //ADC123
  // = PA4; //ADC12
  // = PA5; //ADC12
  // = PA6; //ADC12 LED_BUILTIN_1
  // = PA7; //ADC12 LED_BUILTIN_2
  // ignitionPins[2].pin = PA8;
  // = PA9;  //TXD1=Bluetooth module
  // = PA10; //RXD1=Bluetooth module
  // = PA11; //(DO NOT USE FOR SPEEDUINO) USB
  // = PA12; //(DO NOT USE FOR SPEEDUINO) USB 
  // = PA13;  //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  // = PA14;  //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK
  // = PA15;  //(DO NOT USE FOR SPEEDUINO) NOT ON GPIO - DEBUG ST-LINK

  //******************************************
  //******** PORTB CONNECTIONS *************** 
  //******************************************
  // = PB0;  //(DO NOT USE FOR SPEEDUINO) ADC123 - SPI FLASH CHIP CS pin
  boardDefaultPins.inputs.sensors.pinBaro = PB1; //ADC12
  // = PB2;  //(DO NOT USE FOR SPEEDUINO) BOOT1 
  // = PB3;  //(DO NOT USE FOR SPEEDUINO) SPI1_SCK FLASH CHIP
  // = PB4;  //(DO NOT USE FOR SPEEDUINO) SPI1_MISO FLASH CHIP
  // = PB5;  //(DO NOT USE FOR SPEEDUINO) SPI1_MOSI FLASH CHIP
  // = PB6;  //NRF_CE
  // = PB7;  //NRF_CS
  // = PB8;  //NRF_IRQ
  // ignitionPins[1].pin = PB9; //
  // = PB9;  //
  // = PB10; //TXD3
  // = PB11; //RXD3
  // = PB12; //
  // = PB13;  //SPI2_SCK
  // = PB14;  //SPI2_MISO
  // = PB15;  //SPI2_MOSI

  //******************************************
  //******** PORTC CONNECTIONS *************** 
  //******************************************
  boardDefaultPins.inputs.sensors.pinIAT = PC0; //ADC123 
  boardDefaultPins.inputs.sensors.pinTPS = PC1; //ADC123
  boardDefaultPins.inputs.sensors.pinMAP = PC2; //ADC123 
  boardDefaultPins.inputs.sensors.pinCLT = PC3; //ADC123
  boardDefaultPins.inputs.sensors.pinO2 = PC4; //ADC12
  boardDefaultPins.inputs.sensors.pinBat = PC5;  //ADC12
  boardDefaultPins.outputs.pinBoost = PC6; //
  boardDefaultPins.outputs.pinIdle1 = PC7; //
  // = PC8;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D0
  // = PC9;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D1
  // = PC10;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D2
  // = PC11;  //(DO NOT USE FOR SPEEDUINO) - SDIO_D3
  // = PC12;  //(DO NOT USE FOR SPEEDUINO) - SDIO_SCK
  boardDefaultPins.outputs.pinTachOut = PC13; //
  // = PC14;  //(DO NOT USE FOR SPEEDUINO) - OSC32_IN
  // = PC15;  //(DO NOT USE FOR SPEEDUINO) - OSC32_OUT

  //******************************************
  //******** PORTD CONNECTIONS *************** 
  //******************************************
  // = PD0;  //CANRX
  // = PD1;  //CANTX
  // = PD2;  //(DO NOT USE FOR SPEEDUINO) - SDIO_CMD
  boardDefaultPins.outputs.pinIdle2 = PD3; //
  // = PD4;  //
  boardDefaultPins.inputs.pinFlex = PD4;
  // = PD5; //TXD2
  // = PD6;  //RXD2
  // ignitionPins[0].pin = PD7; //
  // = PD7;  //
  // = PD8;  //
  // ignitionPins[4].pin = PD9;//
  // ignitionPins[3].pin = PD10;//
  // = PD11;  //
  // injectorPins[0].pin = PD12; //
  // injectorPins[1].pin = PD13; //
  // injectorPins[2].pin = PD14; //
  // injectorPins[3].pin = PD15; //

  //******************************************
  //******** PORTE CONNECTIONS *************** 
  //******************************************
  boardDefaultPins.inputs.pinTrigger = PE0; //
  boardDefaultPins.inputs.pinTrigger2 = PE1; //
  boardDefaultPins.outputs.pinStepperEnable = PE2; //
  boardDefaultPins.outputs.pinFuelPump = PE3; //ONBOARD KEY1
  // = PE4;  //ONBOARD KEY2
  boardDefaultPins.outputs.pinStepperStep = PE5; //
  boardDefaultPins.outputs.pinFan = PE6; //
  boardDefaultPins.outputs.pinStepperDir = PE7; //
  // = PE8;  //
  // injectorPins[4].pin = PE9; //
  // = PE10;  //
  // injectorPins[5].pin = PE11; //
  // = PE12; //
  // injectorPins[7].pin = PE13; //
  // injectorPins[6].pin = PE14; //
  // = PE15;  //
#elif (defined(STM32F411xE) || defined(STM32F401xC))
        //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
        //PB2 can't be used as input because is BOOT pin
  static const uint8_t boardFuelPins[] PROGMEM = { PB7, PB6, PB5, PB4 };
  static const uint8_t boardCoilPins[] PROGMEM = { PB9, PB8, PB3, PA15 };
	memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
	memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.sensors.pinTPS = A2;//TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A3; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A0; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A1; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A8; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A4; //Battery reference voltage pin
  boardDefaultPins.inputs.sensors.pinBaro = boardDefaultPins.inputs.sensors.pinMAP;
  boardDefaultPins.outputs.pinTachOut = PB1; //Tacho output pin  (Goes to ULN2803)
  boardDefaultPins.outputs.pinIdle1 = PB2; //Single wire idle control
  boardDefaultPins.outputs.pinIdle2 = PB10; //2 wire idle control
  boardDefaultPins.outputs.pinBoost = PA6; //Boost control
  boardDefaultPins.outputs.pinStepperDir = PB10; //Direction pin  for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = PB2; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinFuelPump = PA8; //Fuel pump output
  boardDefaultPins.outputs.pinFan = PA5; //Pin for the fan output (Goes to ULN2803)

  //external interrupt enabled pins
  boardDefaultPins.inputs.pinFlex = PC14; // Flex sensor (Must be external interrupt enabled)
  boardDefaultPins.inputs.pinTrigger = PC13; //The CAS pin also led pin so bad idea
  boardDefaultPins.inputs.pinTrigger2 = PC15; //The Cam Sensor pin

#elif defined(CORE_STM32)
  //blue pill wiki.stm32duino.com/index.php?title=Blue_Pill
  //Maple mini wiki.stm32duino.com/index.php?title=Maple_Mini
  //pins PA12, PA11 are used for USB or CAN couldn't be used for GPIO
  //PB2 can't be used as input because is BOOT pin
  static const uint8_t boardFuelPins[] PROGMEM = { PB7, PB6, PB5, PB4 };
  static const uint8_t boardCoilPins[] PROGMEM = { PB3, PA15, PA14, PA9, PA8 };
  memcpy_P(boardDefaultPins.outputs.pinInjectors, boardFuelPins, min(sizeof(boardDefaultPins.outputs.pinInjectors), sizeof(boardFuelPins)));
  memcpy_P(boardDefaultPins.outputs.pinCoils, boardCoilPins, min(sizeof(boardDefaultPins.outputs.pinCoils), sizeof(boardCoilPins)));

  boardDefaultPins.inputs.sensors.pinTPS = A0; //TPS input pin
  boardDefaultPins.inputs.sensors.pinMAP = A1; //MAP sensor pin
  boardDefaultPins.inputs.sensors.pinIAT = A2; //IAT sensor pin
  boardDefaultPins.inputs.sensors.pinCLT = A3; //CLS sensor pin
  boardDefaultPins.inputs.sensors.pinO2 = A4; //O2 Sensor pin
  boardDefaultPins.inputs.sensors.pinBat = A5; //Battery reference voltage pin
  boardDefaultPins.inputs.sensors.pinBaro = boardDefaultPins.inputs.sensors.pinMAP;
  boardDefaultPins.outputs.pinIdle1 = PB2; //Single wire idle control
  boardDefaultPins.outputs.pinIdle2 = PA2; //2 wire idle control
  boardDefaultPins.outputs.pinBoost = PA1; //Boost control
  boardDefaultPins.outputs.pinVVT_1 = PA0; //Default VVT output
  boardDefaultPins.outputs.pinVVT_2 = PA2; //Default VVT2 output
  boardDefaultPins.outputs.pinStepperDir = PC15; //Direction pin  for DRV8825 driver
  boardDefaultPins.outputs.pinStepperStep = PC14; //Step pin for DRV8825 driver
  boardDefaultPins.outputs.pinStepperEnable = PC13; //Enable pin for DRV8825
#if defined(ENABLE_DISPLAY_RESET)
  boardDefaultPins.inputs.pinDisplayReset = PB2; // OLED reset pin
#endif
  boardDefaultPins.outputs.pinFan = PB1; //Pin for the fan output
  boardDefaultPins.outputs.pinFuelPump = PB11; //Fuel pump output
  boardDefaultPins.outputs.pinTachOut = PB10; //Tacho output pin
  //external interrupt enabled pins
  boardDefaultPins.inputs.pinFlex = PB8; // Flex sensor (Must be external interrupt enabled)
  boardDefaultPins.inputs.pinTrigger = PA10; //The CAS pin
  boardDefaultPins.inputs.pinTrigger2 = PA13; //The Cam Sensor pin
#endif

  return boardDefaultPins;
}

static inline pin_mapping_t getDefaultPinMapping(uint8_t boardId) {
  switch (boardId)
  {
    //Note: Case 0 (Speeduino v0.1) was removed in Nov 2020 to handle default case for blank FRAM modules

    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
    case 1:
      return getPinMappingsV2_0Shield();
      break;
    #endif

    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
    case 2:
      return getPinMappingsV3_0Shield();
      break;
    #endif

    case 3:
      return getPinMappingsV4_0Shield();
      break;

    #ifndef SMALL_FLASH_MODE
    case 6:
      return getPinMappingsMx5Nb2Shield();
      break;
    #endif

    #ifndef SMALL_FLASH_MODE
    case 8:
      return getPinMappingsMx5Na18Shield();
      break;
    #endif

    #ifndef SMALL_FLASH_MODE
    case 9:
      return getPinMappingsMx5Na16Shield();
      break;
    #endif

    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
    case 10:
      return getPinMappingsTurtanasPcb();
      break;
    #endif

    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
    case 20:
      return getPinMappingsPlazomatV0_1Shield();
      break;
    #endif

    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
    case 30:
      return getPinMappingsDazV6Shield();
      break;
    #endif

   case 31:
      //Pin mappings for the BMW PnP PCBs by pazi88.
      return getPinMappingBmwPnP();
      break;

    #ifndef SMALL_FLASH_MODE
    case 40:
      return getPinMappingsNO2CShield();
      break;
    #endif

    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
    case 41:
      return getPinMappingsUA4CShield();
      break;
    #endif

    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
    case 42:
      return getPinMappingsBlitzboxBL49sp();
      break;
    #endif  
    
    #ifndef SMALL_FLASH_MODE //No support for bluepill here anyway
    case 45:
      return getPinMappingsDiyEfiCore4Shield();
      break;
    #endif  

    case 50:
      return getPinMappingsTeensyRevA();
      break;

    case 51:
      return getPinMappingsTeensyRevB();
      break;

    case 53:
      return getPinMappingsJuiceBox();
      break;

    case 55:
      return getPinMappingsDropBear();
      break;

    case 56:
      return getPinMappingBearCub();
      break;
 
    case 60:
      return getPinMappingsSTM32();
      break;

    default:
    #if !defined(SMALL_FLASH_MODE) //No support for bluepill here anyway
      return getPinMappingsV2_0Shield(); 
    #else
      return getPinMappingsV4_0Shield();
    #endif  
      break;
  }
}

static inline bool isOutputPin(uint8_t pin, const pin_mapping_t &pins) {
  const uint8_t *pOutputPin = (const uint8_t*)(&pins.outputs);
  const uint8_t *pEnd = pOutputPin + NUM_OUTPUT_PINS;
  while (pOutputPin!=pEnd && pin!=*pOutputPin) {
    ++pOutputPin;
  }
  return pOutputPin!=pEnd;  
}

static inline bool pinIsOutput(uint8_t pin, const pin_mapping_t &pins) {
  return isOutputPin(pin, pins)
      //Forbiden or hardware reserved? (Defined at board_xyz.h file)
      || pinIsReserved(pin);
}

static inline bool pinIsSensor(uint8_t pin, const pin_mapping_t &pins) {
  const uint8_t *pSensorPin = (const uint8_t*)(&pins.inputs.sensors);
  const uint8_t *pEnd = pSensorPin + NUM_SENSOR_PINS;
  while (pSensorPin!=pEnd && pin!=*pSensorPin) {
    ++pSensorPin;
  }
  return pSensorPin!=pEnd;  
}

static inline uint8_t overrideDefaultPin(uint8_t boardDefaultPin, uint8_t userEnteredPin) {
  if ( isValidPin(userEnteredPin) ) { 
    return pinTranslate(userEnteredPin);
  }
  return boardDefaultPin;
}

static inline uint8_t overrideDefaultPinAnalog(uint8_t boardDefaultPin, uint8_t userEnteredPin) {
  if ( isValidPin(userEnteredPin) ) { 
    return pinTranslateAnalog(userEnteredPin);
  }
  return boardDefaultPin;  
}

static pin_mapping_t applyOutputPinUserOverrides(pin_mapping_t pins) {
  pins.outputs.pinTachOut = overrideDefaultPin(pins.outputs.pinTachOut, configPage2.tachoPin);
  pins.outputs.pinIgnBypass = overrideDefaultPin(pins.outputs.pinIgnBypass, configPage4.ignBypassPin);
  pins.outputs.pinFuelPump = overrideDefaultPin(pins.outputs.pinFuelPump, configPage4.fuelPumpPin);
  pins.outputs.pinFan = overrideDefaultPin(pins.outputs.pinFan, configPage6.fanPin);
  pins.outputs.pinBoost = overrideDefaultPin(pins.outputs.pinBoost, configPage6.boostPin);
  pins.outputs.pinVVT_1 = overrideDefaultPin(pins.outputs.pinVVT_1, configPage6.vvt1Pin);
  pins.outputs.pinVVT_2 = overrideDefaultPin(pins.outputs.pinVVT_2, configPage10.vvt2Pin);
  pins.outputs.pinWMIEnabled = overrideDefaultPin(pins.outputs.pinWMIEnabled, configPage10.wmiEnabledPin);
  pins.outputs.pinWMIIndicator = overrideDefaultPin(pins.outputs.pinWMIIndicator, configPage10.wmiIndicatorPin);
  pins.outputs.pinIdleUpOutput = overrideDefaultPin(pins.outputs.pinIdleUpOutput, configPage2.idleUpOutputPin);
  pins.outputs.pinAirConComp = overrideDefaultPin(pins.outputs.pinAirConComp, configPage15.airConCompPin);
  pins.outputs.pinAirConFan = overrideDefaultPin(pins.outputs.pinAirConFan, configPage15.airConFanPin);
  pins.outputs.pinResetControl = overrideDefaultPin(pins.outputs.pinResetControl, configPage4.resetControlPin);
  return pins;
}

static inline bool isPinFuel2InputEnabled(void) {
  return configPage10.fuel2Mode == FUEL2_MODE_INPUT_SWITCH;
}
static inline bool isPinSpark2InputEnabled(void) {
  return configPage10.spark2Mode == SPARK2_MODE_INPUT_SWITCH;
}

typedef bool (*enable_func_t)(void);

struct pin_disable_t {
  enable_func_t pEnableFunc;
  size_t pinOffset;
};

static pin_mapping_t conditionalPinDisable(const pin_disable_t pinDisableMap[], uint8_t mapLength, pin_mapping_t pins) {
  for (uint8_t index=0U; index<mapLength; ++index) {
    enable_func_t pEnableFunc = (enable_func_t)pgm_read_ptr(&pinDisableMap[index].pEnableFunc);
    if (!pEnableFunc()) {
      size_t offset = pgm_read_word(&pinDisableMap[index].pinOffset);
      *(uint8_t*)(((byte*)&pins)+offset) = (uint8_t)NOT_A_PIN;
    }
  }
  return pins;
}

// The board can set a default pin for a feature that is not used in the tune.
// By detecting this and setting the pin to NOT_A_PIN, we can free up pins for use
// by other features. E.g. programmable IO
static pin_mapping_t disableUnusedOutputPins(const pin_mapping_t &pins) {
  // Map each feature test function to the pin it controls.
  static const pin_disable_t isEnableMap[] PROGMEM = { //cppcheck-suppress[misra-c2012-17.7]
    { isFanEnabled, offsetof(pin_mapping_t, outputs.pinFan) },
    { isResetControlEnabled, offsetof(pin_mapping_t, outputs.pinResetControl) },
    { isIdlePwm, offsetof(pin_mapping_t, outputs.pinIdle1) },
    { isIdle2PwmEnabled, offsetof(pin_mapping_t, outputs.pinIdle2) },
    { isIdleUpOutputEnabled, offsetof(pin_mapping_t, outputs.pinIdleUpOutput) },
    { isIdleStepper, offsetof(pin_mapping_t, outputs.pinStepperDir) },
    { isIdleStepper, offsetof(pin_mapping_t, outputs.pinStepperStep) },
    { isIdleStepper, offsetof(pin_mapping_t, outputs.pinStepperEnable) },
    { isBoostEnabled, offsetof(pin_mapping_t, outputs.pinBoost) },
    { isVVT_1Enabled, offsetof(pin_mapping_t, outputs.pinVVT_1) },
    { isVVT_2Enabled, offsetof(pin_mapping_t, outputs.pinVVT_2) },
    { isIgnBypassEnabled, offsetof(pin_mapping_t, outputs.pinIgnBypass) },
    { isWMIEnabled, offsetof(pin_mapping_t, outputs.pinWMIEnabled) },
    { isWMIIndicatorEnabled, offsetof(pin_mapping_t, outputs.pinWMIIndicator) },
    { isAirConEnabled, offsetof(pin_mapping_t, outputs.pinAirConComp) },
    { isAirConFanEnabled, offsetof(pin_mapping_t, outputs.pinAirConFan) },
  };
  return conditionalPinDisable(isEnableMap, _countof(isEnableMap), pins);
}

// Allow the user to override the input pins set by the board default
static pin_mapping_t applyInputPinUserOverrides(pin_mapping_t pins) {
  pins.inputs.pinLaunch = overrideDefaultPin(pins.inputs.pinLaunch, configPage6.launchPin);
  pins.inputs.sensors.pinBaro = overrideDefaultPinAnalog(pins.inputs.sensors.pinBaro, configPage6.baroPin);
  pins.inputs.sensors.pinEMAP = overrideDefaultPinAnalog(pins.inputs.sensors.pinEMAP, configPage10.EMAPPin);
  pins.inputs.pinFuel2Input = overrideDefaultPin(pins.inputs.pinFuel2Input, configPage10.fuel2InputPin);
  pins.inputs.pinSpark2Input = overrideDefaultPin(pins.inputs.pinSpark2Input, configPage10.spark2InputPin);
  pins.inputs.pinVSS = overrideDefaultPin(pins.inputs.pinVSS, configPage2.vssPin);
  pins.inputs.sensors.pinFuelPressure = overrideDefaultPinAnalog(pins.inputs.sensors.pinFuelPressure, configPage10.fuelPressurePin);
  pins.inputs.sensors.pinOilPressure = overrideDefaultPinAnalog(pins.inputs.sensors.pinOilPressure, configPage10.oilPressurePin);
  pins.inputs.pinWMIEmpty = overrideDefaultPin(pins.inputs.pinWMIEmpty, configPage10.wmiEmptyPin);
#ifdef SD_LOGGING  
  pins.inputs.pinSDEnable = overrideDefaultPin(pins.inputs.pinSDEnable, configPage13.onboard_log_tr5_Epin_pin);
#endif
  pins.inputs.pinIdleUp = overrideDefaultPin(pins.inputs.pinIdleUp, configPage2.idleUpPin);
  pins.inputs.pinCTPS = overrideDefaultPin(pins.inputs.pinCTPS, configPage2.CTPSPin);
  pins.inputs.pinAirConRequest = overrideDefaultPin(pins.inputs.pinAirConRequest, configPage15.airConReqPin);
  return pins;
}

// The board can set a default pin for a feature that is not used in the tune.
// By detecting this and setting the pin to NOT_A_PIN, we can free up pins for use
// by other features. E.g. programmable IO
static pin_mapping_t disableUnusedInputPins(const pin_mapping_t &pins) {
  static const pin_disable_t isEnableMap[] PROGMEM = { //cppcheck-suppress[misra-c2012-17.7]
    { isClutchTriggerEnabled, offsetof(pin_mapping_t, inputs.pinLaunch) },
    { isExtBaroEnabled, offsetof(pin_mapping_t, inputs.sensors.pinBaro) },
    { isExhMAPEnabled, offsetof(pin_mapping_t, inputs.sensors.pinEMAP) },
    { isPinFuel2InputEnabled, offsetof(pin_mapping_t, inputs.pinFuel2Input) },
    { isPinSpark2InputEnabled, offsetof(pin_mapping_t, inputs.pinSpark2Input) },
    { isVssModeInterrupt, offsetof(pin_mapping_t, inputs.pinVSS) },
    { isFuelPressureEnabled, offsetof(pin_mapping_t, inputs.sensors.pinFuelPressure) },
    { isOilPressureEnabled, offsetof(pin_mapping_t, inputs.sensors.pinOilPressure) },
    { isWMIEmptyEnabled, offsetof(pin_mapping_t, inputs.pinWMIEmpty) },
#ifdef SD_LOGGING  
    { isSDLoggingEnabled, offsetof(pin_mapping_t, inputs.pinSDEnable) },
#endif
    { isIdleUpEnabled, offsetof(pin_mapping_t, inputs.pinIdleUp) },
    { isCTPSEnabled, offsetof(pin_mapping_t, inputs.pinCTPS) },
    { isAirConEnabled, offsetof(pin_mapping_t, inputs.pinAirConRequest) },
    { isFlexEnabled, offsetof(pin_mapping_t, inputs.pinFlex) },
  };
  return conditionalPinDisable(isEnableMap, _countof(isEnableMap), pins);
}

// Disables any input pin that overlaps with an output pin
static pin_mapping_t disableInputOutputOverlapPins(pin_mapping_t pins) {
  uint8_t *pInputPin = (uint8_t*)(&pins.inputs);
  const uint8_t *pEnd = pInputPin + NUM_INPUT_PINS;
  while (pInputPin!=pEnd) {
    if (isValidPin(*pInputPin) && pinIsOutput(*pInputPin, pins)) {
      *pInputPin = NOT_A_PIN;
    }
    ++pInputPin;
  }
  return pins;
}

pin_mapping_t pinMapping;

/** Set board / microcontroller specific pin mappings / assignments.
 * The boardID is switch-case compared against raw boardID integers (not enum or defined label, and probably no need for that either)
 * which are originated from tuning SW (e.g. TS) set values and are available in reference/speeduino.ini (See pinLayout, note also that
 * numbering is not contiguous here).
 */
void setPinMapping(byte boardID)
{
  pinMapping = disableInputOutputOverlapPins(
                  disableUnusedInputPins(
                    applyInputPinUserOverrides(
                      disableUnusedOutputPins(
                          applyOutputPinUserOverrides(
                              getDefaultPinMapping(boardID))))));
}

bool pinIsUsed(uint8_t pin, const pin_mapping_t &pins)
{
  return 
    //Analog input?
    pinIsSensor(pin, pins) 
    //Functions?
    || pinIsOutput(pin, pins);
}
