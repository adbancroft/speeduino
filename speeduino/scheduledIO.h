/** @file
 * @brief Injector and Coil (toggle/open/close) control (under various situations, eg with particular cylinder count, rotary engine type or wasted spark ign, etc.).
 * Also accounts for presence of MC33810 injector/ignition (dwell, etc.) control circuit.
 */

#ifndef SCHEDULEDIO_H
#define SCHEDULEDIO_H

#include "board_definition.h"
#include "timers.h"

// ================================= Injection ================================= 

/** @brief An injector action. The parameter is **one** based index of the injector. I.e. the injector channel number */
using injectorAction = void(*)(uint8_t);

/**
 * @brief Injector controller function pack.
 * 
 * Some boards use an injector control system other than board pins directly controlling the injectors. 
 */
struct injector_control_t {
    injectorAction openInjector;    ///< Open the injector
    injectorAction closeInjector;   ///< Close the injector
    injectorAction toggleInjector;  ///< Opne to Close, Close to Open the injector
};

/**
 * @brief Inject the injector control functions
 * 
 * @param control 
 */
void setInjectorControlActions(const injector_control_t &control);

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
    extern injector_control_t injectorControl;
    injectorControl.openInjector(channelIndex);
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
    extern injector_control_t injectorControl;
    injectorControl.closeInjector(channelIndex);
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
    extern injector_control_t injectorControl;
    injectorControl.toggleInjector(channelIndex);
}


// ================================= Ignition ================================= 

/** @brief An ignition coil action. The parameter is **one** based index of the coil. I.e. the coil channel number */
using coilAction = void(*)(uint8_t);

/**
 * @brief Ignition coil function pack.
 * 
 * Some boards use a coil control system other than board pins directly controlling the coils. 
 */
struct coil_control_t {
    coilAction beginCoilCharge; ///< Start charging the coil
    coilAction endCoilCharge;   ///< Discharge the coil. I.e.spark 
};

/**
 * @brief Inject the coil control functions
 * 
 * @param control 
 */
void setIgnitionControlActions(const coil_control_t &control);

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
    extern coil_control_t coilControl;
    coilControl.beginCoilCharge(channelIndex);
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
    extern coil_control_t coilControl;
    coilControl.endCoilCharge(channelIndex);
    if(configPage6.tachoMode) { 
        TACHO_PULSE_HIGH(); 
    }
}

#endif
