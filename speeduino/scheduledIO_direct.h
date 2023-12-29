#pragma once

#include "globals.h"
#include "port_pin.h"

//These are for the direct port manipulation of the injectors, coils
extern ioPort injectorPins[INJ_CHANNELS];
extern ioPort ignitionPins[IGN_CHANNELS];

//Functions are used to define how each injector control system functions. These are then called by the master openInjectx() function.
//The DIRECT functions (ie individual pins) are defined below. Others should be defined in their relevant acc_x.h file

static inline void openInjector_DIRECT(uint8_t channelIndex) { setPin_High(injectorPins[channelIndex-1U]); }
static inline void closeInjector_DIRECT(uint8_t channelIndex) { setPin_Low(injectorPins[channelIndex-1U]); }
static inline void toggleInjector_DIRECT(uint8_t channelIndex) { togglePin(injectorPins[channelIndex-1U]); }

// ======================== Ignition ========================

static inline void coilStartCharging_DIRECT(uint8_t channelIndex) {  
    setPinState(ignitionPins[channelIndex-1U], (configPage4.IgInv == GOING_HIGH ? LOW : HIGH));
}
static inline void coilStopCharging_DIRECT(uint8_t channelIndex) { 
    setPinState(ignitionPins[channelIndex-1U], (configPage4.IgInv == GOING_HIGH ? HIGH : LOW));
}
