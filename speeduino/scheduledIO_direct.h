#pragma once

#include "globals.h"
#include "port_pin.h"

//Functions are used to define how each injector control system functions. These are then called by the master openInjectx() function.
//The DIRECT functions (ie individual pins) are defined below. Others should be defined in their relevant acc_x.h file
/**
 * @brief Setup the injector (fuel) pins
 * 
 * @param pins Pin numbers in cylinder order. *Assumption is that the array has at least IGN_CHANNELS elements*
 */
void initialiseInjectorPins_DIRECT(const uint8_t pins[]);

static inline void openInjector_DIRECT(uint8_t channelIndex) { extern ioPort injectorPins[INJ_CHANNELS]; setPin_High(injectorPins[channelIndex-1U]); }
static inline void closeInjector_DIRECT(uint8_t channelIndex) { extern ioPort injectorPins[INJ_CHANNELS]; setPin_Low(injectorPins[channelIndex-1U]); }
static inline void toggleInjector_DIRECT(uint8_t channelIndex) { extern ioPort injectorPins[INJ_CHANNELS]; togglePin(injectorPins[channelIndex-1U]); }

// ======================== Ignition ========================

/**
 * @brief Setup the ignition (coil) pins
 * 
 * @param pins Pin numbers in cylinder order. *Assumption is that the array has at least IGN_CHANNELS elements*
 */
void initialiseIgnitionPins_DIRECT(const uint8_t pins[]);

static inline void coilStartCharging_DIRECT(uint8_t channelIndex) {  
    extern ioPort ignitionPins[IGN_CHANNELS];
    setPinState(ignitionPins[channelIndex-1U], (configPage4.IgInv == GOING_HIGH ? LOW : HIGH));
}
static inline void coilStopCharging_DIRECT(uint8_t channelIndex) { 
    extern ioPort ignitionPins[IGN_CHANNELS];
    setPinState(ignitionPins[channelIndex-1U], (configPage4.IgInv == GOING_HIGH ? HIGH : LOW));
}
