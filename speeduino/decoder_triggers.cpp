#include <Arduino.h>
#include "decoder_triggers.h"
#include "decoders.h"
#include "port_pin.h"
#include "globals.h"
#include "auxiliaries.h"

static constexpr trigger_t NULL_TRIGGER = { nullptr, 0U };
static uint16_t nullGetRPM(void){return 0U;} //initialisation function for getRpm, returns safe value of 0
static int nullGetCrankAngle(void){return 0;} //initialisation function for getCrankAngle, returns safe value of 0

// ========================== External Global Variables ========================== 

decoder_t decoder = { 
    nullGetRPM, 
    nullGetCrankAngle,
    triggerSetEndTeeth_missingTooth,
    NULL_TRIGGER, 
    NULL_TRIGGER, 
    NULL_TRIGGER
};

// ========================== Trigger Definition ========================== 

static inline uint8_t getConfigPrimaryTriggerMode(void) {
  return configPage4.TrigEdge == 0U  ? RISING : FALLING;
}

static inline uint8_t getConfigSecondaryTriggerMode(void) {
  return configPage4.TrigEdgeSec == 0U  ? RISING : FALLING;
}

static inline uint8_t getConfigTertiaryTriggerMode(void) {
  return configPage10.TrigEdgeThrd == 0U  ? RISING : FALLING;
}

static decoder_t getDecoder(uint8_t pattern) {
  switch (pattern)
  {
    case DECODER_MISSING_TOOTH:
      //Missing tooth decoder
      {
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

        triggerSetup_missingTooth();
        decoder_t localDecoder = {
            getRPM_missingTooth,
            getCrankAngle_missingTooth,
            triggerSetEndTeeth_missingTooth,
            { triggerPri_missingTooth, getConfigPrimaryTriggerMode() },
            NULL_TRIGGER,
            NULL_TRIGGER,
        };            
        if(BIT_CHECK(decoderState, BIT_DECODER_HAS_SECONDARY)) { 
          localDecoder.secondaryTrigger = { triggerSec_missingTooth, getConfigSecondaryTriggerMode() }; 
        }
        if(isVVT_2Enabled()) { 
          localDecoder.tertiaryTrigger = { triggerThird_missingTooth, getConfigTertiaryTriggerMode() }; // we only need this for vvt2, so not really needed if it's not used
        }
        return localDecoder;
      }
      break;

    case DECODER_BASIC_DISTRIBUTOR:
      // Basic distributor
      triggerSetup_BasicDistributor();
      return {
        getRPM_BasicDistributor,
        getCrankAngle_BasicDistributor,
        triggerSetEndTeeth_BasicDistributor,
        { triggerPri_BasicDistributor, getConfigPrimaryTriggerMode() },
        NULL_TRIGGER,
        NULL_TRIGGER,
        };
      break;

    case 2:
      triggerSetup_DualWheel();
      return {
        getRPM_DualWheel,
        getCrankAngle_DualWheel,
        triggerSetEndTeeth_DualWheel,
        { triggerPri_DualWheel, getConfigPrimaryTriggerMode() },
        { triggerSec_DualWheel, getConfigSecondaryTriggerMode() },
        NULL_TRIGGER,
  		};
      break;

    case DECODER_GM7X:
      triggerSetup_GM7X();
      return {
        getRPM_GM7X,
        getCrankAngle_GM7X,
        triggerSetEndTeeth_GM7X,
        { triggerPri_GM7X, getConfigPrimaryTriggerMode() },
        NULL_TRIGGER,
        NULL_TRIGGER,
      };
      break;

    case DECODER_4G63:
      triggerSetup_4G63();
      return {
        getRPM_4G63,
        getCrankAngle_4G63,
        triggerSetEndTeeth_4G63,
        { triggerPri_4G63, CHANGE },
        { triggerSec_4G63, FALLING },
        NULL_TRIGGER,
      };
      break;

    case DECODER_24X:
      triggerSetup_24X();
      return {
        getRPM_24X,
        getCrankAngle_24X,
        triggerSetEndTeeth_24X,
        { triggerPri_24X, getConfigPrimaryTriggerMode() },
        { triggerSec_24X, CHANGE }, //Secondary is always on every change
        NULL_TRIGGER,
      };
      break;

    case DECODER_JEEP2000:
      triggerSetup_Jeep2000();
      return {
        getRPM_Jeep2000,
        getCrankAngle_Jeep2000,
        triggerSetEndTeeth_Jeep2000,
        { triggerPri_Jeep2000, getConfigPrimaryTriggerMode() },
        { triggerSec_Jeep2000, CHANGE },
        NULL_TRIGGER,
      };
      break;

    case DECODER_AUDI135:
      triggerSetup_Audi135();
      return {
        getRPM_Audi135,
        getCrankAngle_Audi135,
        triggerSetEndTeeth_Audi135,
        { triggerPri_Audi135, getConfigPrimaryTriggerMode() },
        { triggerSec_Audi135, RISING }, //always rising for this trigger
        NULL_TRIGGER,
      };
      break;

    case DECODER_HONDA_D17:
      triggerSetup_HondaD17();
      return {
        getRPM_HondaD17,
        getCrankAngle_HondaD17,
        triggerSetEndTeeth_HondaD17,
        { triggerPri_HondaD17, getConfigPrimaryTriggerMode() },
        { triggerSec_HondaD17, CHANGE },
        NULL_TRIGGER,
      };
      break;

    case DECODER_MIATA_9905:
      triggerSetup_Miata9905();
      return {
        getRPM_Miata9905,
        getCrankAngle_Miata9905,
        triggerSetEndTeeth_Miata9905,
        { triggerPri_Miata9905, getConfigPrimaryTriggerMode() },
        { triggerSec_Miata9905, getConfigSecondaryTriggerMode() }, 
        NULL_TRIGGER,
      };
      break;

    case DECODER_MAZDA_AU:
      triggerSetup_MazdaAU();
      return {
        getRPM_MazdaAU,
        getCrankAngle_MazdaAU,
        triggerSetEndTeeth_MazdaAU,
        { triggerPri_MazdaAU, getConfigPrimaryTriggerMode() },
        { triggerSec_MazdaAU, FALLING },
        NULL_TRIGGER,
      };
      break;

    case DECODER_NON360:
      triggerSetup_non360();
      return {
        getRPM_non360,
        getCrankAngle_non360,
        triggerSetEndTeeth_non360,
        { triggerPri_DualWheel, getConfigPrimaryTriggerMode() }, //Is identical to the dual wheel decoder, so that is used. Same goes for the secondary below
        { triggerSec_DualWheel, FALLING }, //Note the use of the Dual Wheel trigger function here. No point in having the same code in twice.
        NULL_TRIGGER,
      };
      break;

    case DECODER_NISSAN_360:
      triggerSetup_Nissan360();
      return {
        getRPM_Nissan360,
        getCrankAngle_Nissan360,
        triggerSetEndTeeth_Nissan360,       
        { triggerPri_Nissan360, getConfigPrimaryTriggerMode() },
        { triggerSec_Nissan360, CHANGE },
        NULL_TRIGGER,
      };
      break;

    case DECODER_SUBARU_67:
      triggerSetup_Subaru67();
      return {
        getRPM_Subaru67,
        getCrankAngle_Subaru67,
        triggerSetEndTeeth_Subaru67,
        { triggerPri_Subaru67, getConfigPrimaryTriggerMode() },
        { triggerSec_Subaru67, FALLING },
        NULL_TRIGGER,
      };
      break;

    case DECODER_DAIHATSU_PLUS1:
      triggerSetup_Daihatsu();
      return {
        getRPM_Daihatsu,
        getCrankAngle_Daihatsu,
        triggerSetEndTeeth_Daihatsu,
        { triggerPri_Daihatsu, getConfigPrimaryTriggerMode() },
        NULL_TRIGGER,
        NULL_TRIGGER,
  		};
      break;

    case DECODER_HARLEY:
      triggerSetup_Harley();
      return {
        getRPM_Harley,
        getCrankAngle_Harley,
        triggerSetEndTeeth_Harley,
        { triggerPri_Harley, RISING }, //Always rising
        NULL_TRIGGER,
        NULL_TRIGGER,
  		};
      break;

    case DECODER_36_2_2_2:
      //36-2-2-2
      triggerSetup_ThirtySixMinus222();
      return {
        getRPM_ThirtySixMinus222,
        getCrankAngle_missingTooth, //This uses the same function as the missing tooth decoder, so no need to duplicate cod,
        triggerSetEndTeeth_ThirtySixMinus222,
        { triggerPri_ThirtySixMinus222, getConfigPrimaryTriggerMode() },
        { triggerSec_ThirtySixMinus222, getConfigSecondaryTriggerMode() }, 
        NULL_TRIGGER,
      };
      break;

    case DECODER_36_2_1:
      //36-2-1
      triggerSetup_ThirtySixMinus21();
      return {
        getRPM_ThirtySixMinus21,
        getCrankAngle_missingTooth, //This uses the same function as the missing tooth decoder, so no need to duplicate cod,
        triggerSetEndTeeth_ThirtySixMinus21,
        { triggerPri_ThirtySixMinus21, getConfigPrimaryTriggerMode() },
        { triggerSec_missingTooth, getConfigSecondaryTriggerMode() }, 
        NULL_TRIGGER,
      };
      break;

    case DECODER_420A:
      //DSM 420a
      triggerSetup_420a();
      return {
        getRPM_420a,
        getCrankAngle_420a,
        triggerSetEndTeeth_420a,
        { triggerPri_420a, getConfigPrimaryTriggerMode() },
        { triggerSec_420a, FALLING }, //Always falling edge
        NULL_TRIGGER,
      };
      break;

    case DECODER_WEBER:
      //Weber-Marelli
      triggerSetup_DualWheel();
      return {
        getRPM_DualWheel,
        getCrankAngle_DualWheel,
        triggerSetEndTeeth_DualWheel,
        { triggerPri_Webber, getConfigPrimaryTriggerMode() },
        { triggerSec_Webber, getConfigSecondaryTriggerMode() }, 
        NULL_TRIGGER,
      };
      break;

    case DECODER_ST170:
      //Ford ST170
      triggerSetup_FordST170();
      return {
        getRPM_FordST170,
        getCrankAngle_FordST170,
        triggerSetEndTeeth_FordST170,
        { triggerPri_missingTooth, getConfigPrimaryTriggerMode() },
        { triggerSec_FordST170, getConfigSecondaryTriggerMode() }, 
        NULL_TRIGGER,
      };
      break;
    
    case DECODER_DRZ400:
      triggerSetup_DRZ400();
      return {
        getRPM_DualWheel,
        getCrankAngle_DualWheel,
        triggerSetEndTeeth_DualWheel,
        { triggerPri_DualWheel, getConfigPrimaryTriggerMode() },
        { triggerSec_DRZ400, getConfigSecondaryTriggerMode() }, 
        NULL_TRIGGER,
      };
      break;
    
    case DECODER_NGC:
      //Chrysler NGC - 4, 6 and 8 cylinder
      triggerSetup_NGC();      
      return {
        getRPM_NGC,
        getCrankAngle_missingTooth,
        triggerSetEndTeeth_NGC,
        { triggerPri_NGC, CHANGE },
        (configPage2.nCylinders == 4U ? trigger_t { triggerSec_NGC4, CHANGE } : trigger_t { triggerSec_NGC68, FALLING }),
        NULL_TRIGGER
      };
      break;
    
    case DECODER_VMAX:
      triggerSetup_Vmax();
      return {
        getRPM_Vmax,
        getCrankAngle_Vmax,
        triggerSetEndTeeth_Vmax,
        { triggerPri_Vmax, CHANGE }, 
        NULL_TRIGGER,
        NULL_TRIGGER,
      };
      break;
    
    case DECODER_RENIX:
      //Renault 44 tooth decoder
      triggerSetup_Renix();
      return {
        getRPM_missingTooth,
        getCrankAngle_missingTooth,
        triggerSetEndTeeth_Renix,
        { triggerPri_Renix, getConfigPrimaryTriggerMode() },
        NULL_TRIGGER,
        NULL_TRIGGER,
      };
      break;
    
    case DECODER_ROVERMEMS:
      //Rover MEMs - covers multiple flywheel trigger combinations.
      triggerSetup_RoverMEMS();
      return {
        getRPM_RoverMEMS,
        getCrankAngle_missingTooth,
        triggerSetEndTeeth_RoverMEMS,
        { triggerPri_RoverMEMS, getConfigPrimaryTriggerMode() },
        { triggerSec_RoverMEMS, getConfigSecondaryTriggerMode() }, 
        NULL_TRIGGER,
      };
      break;   

    case DECODER_SUZUKI_K6A:
      triggerSetup_SuzukiK6A();
      return {
        getRPM_SuzukiK6A,
        getCrankAngle_SuzukiK6A,
        triggerSetEndTeeth_SuzukiK6A,
        { triggerPri_SuzukiK6A, getConfigPrimaryTriggerMode() },
        NULL_TRIGGER,
        NULL_TRIGGER,
      };
      break;

    default:
      return {
        getRPM_missingTooth,
        getCrankAngle_missingTooth,
        triggerSetEndTeeth_missingTooth,
        { triggerPri_missingTooth, getConfigPrimaryTriggerMode() },
        NULL_TRIGGER,
        NULL_TRIGGER,
      };
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