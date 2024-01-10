#include <Arduino.h>
#include "decoder_triggers.h"
#include "decoders.h"
#include "port_pin.h"
#include "globals.h"
#include "auxiliaries.h"

static uint16_t nullGetRPM(void){return 0U;} //initialisation function for getRpm, returns safe value of 0
static int nullGetCrankAngle(void){return 0;} //initialisation function for getCrankAngle, returns safe value of 0
static void nullSetEndTeeth(void) { return; }
static constexpr decoder_t NULL_DECODER = { 
  nullGetRPM, 
  nullGetCrankAngle,
  nullSetEndTeeth,
  NULL_TRIGGER, 
  NULL_TRIGGER, 
  NULL_TRIGGER
 };

// ========================== Trigger Definition ========================== 

static decoder_t getDecoder(uint8_t pattern) {
  switch (pattern)
  {
    case DECODER_MISSING_TOOTH:
      //set the secondary trigger edge automatically to correct working value with poll level mode to 
      //enable cam angle detection in closed loop vvt.
      if (configPage4.trigPatternSec == SEC_TRIGGER_POLL) {
        //Explanation: currently cam trigger for VVT is only captured when revolution one == 1. So we 
        //need to make sure that the edge trigger happens on the first revolution. So now when we set
        //the poll level to be low on revolution one and it's checked at tooth #1. This means that the
        //cam signal needs to go high during the first revolution to be high on next revolution at tooth #1.
        //So poll level low = cam trigger edge rising.
        configPage4.TrigEdgeSec = configPage4.PollLevelPolarity; 
      }
      return triggerSetup_missingTooth();
      break;

    case DECODER_BASIC_DISTRIBUTOR:
      return triggerSetup_BasicDistributor();
      break;

    case DECODER_DUAL_WHEEL:
      return triggerSetup_DualWheel();
      break;

    case DECODER_GM7X:
      return triggerSetup_GM7X();
      break;

    case DECODER_4G63:
      return triggerSetup_4G63();
      break;

    case DECODER_24X:
      return triggerSetup_24X();
      break;

    case DECODER_JEEP2000:
      return triggerSetup_Jeep2000();
      break;

    case DECODER_AUDI135:
      return triggerSetup_Audi135();
      break;

    case DECODER_HONDA_D17:
      return triggerSetup_HondaD17();
      break;
      
    case DECODER_HONDA_J32:
      return triggerSetup_HondaJ32();
      break;

    case DECODER_MIATA_9905:
      return triggerSetup_Miata9905();
      break;

    case DECODER_MAZDA_AU:
      return triggerSetup_MazdaAU();
      break;

    case DECODER_NON360:
      return triggerSetup_non360();
      break;

    case DECODER_NISSAN_360:
      return triggerSetup_Nissan360();
      break;

    case DECODER_SUBARU_67:
      return triggerSetup_Subaru67();
      break;

    case DECODER_DAIHATSU_PLUS1:
      return triggerSetup_Daihatsu();
      break;

    case DECODER_HARLEY:
      return triggerSetup_Harley();
      break;

    case DECODER_36_2_2_2:
      return triggerSetup_ThirtySixMinus222();
      break;

    case DECODER_36_2_1:
      return triggerSetup_ThirtySixMinus21();
      break;

    case DECODER_420A:
      return triggerSetup_420a();
      break;

    case DECODER_WEBER:
      return triggerSetup_Webber();
      break;

    case DECODER_ST170:
      return triggerSetup_FordST170();
      break;
    
    case DECODER_DRZ400:
      return triggerSetup_DRZ400();
      break;
    
    case DECODER_NGC:
      return triggerSetup_NGC();      
      break;
    
    case DECODER_VMAX:
      return triggerSetup_Vmax();
      break;
    
    case DECODER_RENIX:
      return triggerSetup_Renix();
      break;
    
    case DECODER_ROVERMEMS:
      return triggerSetup_RoverMEMS();
      break;   

    case DECODER_SUZUKI_K6A:
      return triggerSetup_SuzukiK6A();
      break;

    default:
      return NULL_DECODER;
      break;
  }
}

// ========================== I/O ========================== 

#if defined(CORE_AVR)
using trigger_ioport_t = ioPort; // Use direct port read on AVR (faster)
static inline trigger_ioport_t createIoPort(uint8_t pin) {
  return pinToInputPort(pin);
}
#else
using trigger_ioport_t = uint8_t; // "Normal" digital pin read on non-AVR cores, which are generally a lot faster
static inline trigger_ioport_t createIoPort(uint8_t pin) {
  return pin;
}
#endif

static trigger_ioport_t primaryIoPort;
static trigger_ioport_t secondaryIoPort;
static trigger_ioport_t tertiaryIoPort;

// ========================== Interrupts ========================== 

static void attachInterrupt(uint8_t pin, const trigger_t &trigger) {
  if (isValidPin(pin)) {
    detachInterrupt( digitalPinToInterrupt(pin) );
    if (isValid(trigger)) {
      attachInterrupt( digitalPinToInterrupt(pin), trigger.handler, trigger.edge );
    }
  }
}

// ========================== Initialization Internal ========================== 

static inline uint8_t validatePin(uint8_t pin, const trigger_t &trigger) {
  if (isValid(trigger)) {
    return pin;
  }
  return NOT_A_PIN;
}

static trigger_ioport_t createTrigger(uint8_t pin, const trigger_t &trigger) {
  pin = validatePin(pin, trigger);
  attachInterrupt(pin, trigger);
  return createIoPort(pin);
}

static void initialiseDecoder(uint8_t pattern, uint8_t pinPrimary, uint8_t pinSecondary, uint8_t pinTertiary) {
  decoder = getDecoder(pattern);

  primaryIoPort = createTrigger(pinPrimary, decoder.primaryTrigger);
  secondaryIoPort = createTrigger(pinSecondary, decoder.secondaryTrigger);
  tertiaryIoPort = createTrigger(pinTertiary, decoder.tertiaryTrigger);

#if defined(CORE_TEENSY41)
  //Teensy 4 requires a HYSTERESIS flag to be set on the trigger pins to prevent false interrupts
  setTriggerHysteresis(pinPrimary, pinSecondary);
#endif
}

// ========================== External ========================== 

decoder_t decoder = NULL_DECODER;

void initialiseDecoder(void) {
  initialiseDecoder(configPage4.TrigPattern, pinTrigger, pinTrigger2, pinTrigger3);

  if (!isValid(decoder.secondaryTrigger)) {
    pinTrigger2 = NOT_A_PIN;
  }
  if (!isValid(decoder.tertiaryTrigger)) {
    pinTrigger3 = NOT_A_PIN;
  }
}


void attachPrimaryInterrupt(const trigger_t &trigger) {
  attachInterrupt(pinTrigger, trigger);
}

void attachSecondaryInterrupt(const trigger_t &trigger) {
  attachInterrupt(pinTrigger2, trigger);
}

void attachTertiaryInterrupt(const trigger_t &trigger) {
  attachInterrupt(pinTrigger3, trigger);
}

uint8_t getTriggerPinState(const trigger_t &trigger) {
  if (&trigger==&decoder.primaryTrigger) {
    return readPin(primaryIoPort);
  }
  if (&trigger==&decoder.secondaryTrigger) {
    return readPin(secondaryIoPort);
  }
  return readPin(tertiaryIoPort);
}