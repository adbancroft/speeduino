/** @file
 * @brief Injector and Coil (toggle/open/close) control (under various situations, eg with particular cylinder count, rotary engine type or wasted spark ign, etc.).
 * Also accounts for presence of MC33810 injector/ignition (dwell, etc.) control circuit.
 */

#ifndef SCHEDULEDIO_H
#define SCHEDULEDIO_H

#include "board_definition.h"
#include "scheduledIO_direct.h"
#include "scheduledIO_MC33810.h"
#include "timers.h"

/** \enum ScheduleOutputControl
 * @brief Controls how coil & injection pins are controlled
 * */
enum ScheduleOutputControl {
    /** Directly via an IO pin */
    OUTPUT_CONTROL_DIRECT = 0U,
    /** Via an external MC33810 module */
    OUTPUT_CONTROL_MC33810 = 10U
};

// ================================= Injection ================================= 

#if defined(OUTPUT_CONTROL_SUPPORTED)
extern ScheduleOutputControl injectorOutputControl;
#endif

/**
 * @brief Setup the injector (fuel) pins and output control method
 * 
 * @param pins Pin numbers in cylinder order. *Assumption is that the array has at least IGN_CHANNELS elements*
 */
void initialiseInjectorPins(const uint8_t pins[]);

/**
 * @brief Open an injector
 * 
 * @param channelIndex **One** based index of the injector. I.e. the injector channel number
 */
static inline void openInjector(uint8_t channelIndex) {
#if defined(OUTPUT_CONTROL_SUPPORTED)
    if(injectorOutputControl != OUTPUT_CONTROL_MC33810) {
        openInjector_DIRECT(channelIndex);
    } else {
        openInjector_MC33810(channelIndex);
    }
#else
    openInjector_DIRECT(channelIndex);
#endif
    if (channelIndex-1U<=BIT_STATUS1_INJ4) { 
        BIT_SET(currentStatus.status1, BIT_STATUS1_INJ1+(channelIndex)-1U); 
    }
}

/**
 * @brief Close an injector
 * 
 * @param channelIndex **One** based index of the injector. I.e. the injector channel number
 */
static inline void closeInjector(uint8_t channelIndex) {
#if defined(OUTPUT_CONTROL_SUPPORTED)
    if(injectorOutputControl != OUTPUT_CONTROL_MC33810) {
        closeInjector_DIRECT(channelIndex);
    } else {
        closeInjector_MC33810(channelIndex);
    }
#else
    closeInjector_DIRECT(channelIndex);
#endif
    if (channelIndex-1U<=BIT_STATUS1_INJ4) { 
        BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ1+(channelIndex)-1U);
    }
}

/**
 * @brief Toggle injector: closed to open or open to closed.
 * 
 * @param channelIndex **One** based index of the injector. I.e. the injector channel number
 */
static inline void toggleInjector(uint8_t channelIndex) {
#if defined(OUTPUT_CONTROL_SUPPORTED)
    if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { 
        toggleInjector_DIRECT(channelIndex);
    } else {
        toggleInjector_MC33810(channelIndex);
    }
#else
    toggleInjector_DIRECT(channelIndex);
#endif
}


// ================================= Ignition ================================= 

#if defined(OUTPUT_CONTROL_SUPPORTED)
extern ScheduleOutputControl ignitionOutputControl; //Specifies whether the coils are controlled directly (Via an IO pin) or using something like the MC33810
#endif

/**
 * @brief Setup the ignition (coil) pins and output control method
 * 
 * @param pins Pin numbers in cylinder order. *Assumption is that the array has at least IGN_CHANNELS elements*
 */
void initialiseIgnitionPins(const uint8_t pins[]);

/**
 * @brief Start charging a coil
 * 
 * @param channelIndex **One** based index of the coil. I.e. the coil channel number
 */
static inline void beginCoilCharge(uint8_t channelIndex) {
#if defined(OUTPUT_CONTROL_SUPPORTED)
    if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { 
        coilStartCharging_DIRECT(channelIndex);
    } else { 
        coilStartCharging_MC33810(channelIndex);
    }
#else
    coilStartCharging_DIRECT(channelIndex); 
#endif
    if(configPage6.tachoMode) { 
        TACHO_PULSE_LOW(); 
    } else { 
        tachoOutputFlag = READY; 
    }
}

/**
 * @brief Fire a spark
 * 
 * @param channelIndex **One** based index of the coil. I.e. the coil channel number
 */
static inline void endCoilCharge(uint8_t channelIndex) {
#if defined(OUTPUT_CONTROL_SUPPORTED)
    if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { 
        coilStopCharging_DIRECT(channelIndex);
    } else { 
        coilStopCharging_MC33810(channelIndex); 
    }
#else
    coilStopCharging_DIRECT(channelIndex);
#endif
    if(configPage6.tachoMode) { 
        TACHO_PULSE_HIGH(); 
    }
}

#endif
