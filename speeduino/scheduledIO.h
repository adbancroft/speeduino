/** @file
 * Injector and Coil (toggle/open/close) control (under various situations, eg with particular cylinder count, rotary engine type or wasted spark ign, etc.).
 * Also accounts for presence of MC33810 injector/ignition (dwell, etc.) control circuit.
 * Functions here are typically assigned (at initialisation) to callback function variables (e.g. inj1StartFunction or inj1EndFunction) 
 * form where they are called (by scheduler.ino).
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

static inline void nullCallback(void) { return; }

typedef void (*voidVoidCallback)(void);

// ================================= Injection ================================= 

#if defined(OUTPUT_CONTROL_SUPPORTED)
extern ScheduleOutputControl injectorOutputControl;
#endif

/**
 * @brief Register an injector pin.
 * 
 * @param pin The pin number
 * @return ioPort 
 */
ioPort registerInjectorPin(uint8_t pin);

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

static inline void openInjector1(void)   { openInjector(1); }
static inline void closeInjector1(void)  { closeInjector(1); }
static inline void openInjector2(void)   { openInjector(2); }
static inline void closeInjector2(void)  { closeInjector(2); }
static inline void openInjector3(void)   { openInjector(3); }
static inline void closeInjector3(void)  { closeInjector(3); }
static inline void openInjector4(void)   { openInjector(4); }
static inline void closeInjector4(void)  { closeInjector(4); }
static inline void openInjector5(void)   { openInjector(5); }
static inline void closeInjector5(void)  { closeInjector(5); }
static inline void openInjector6(void)   { openInjector(6); }
static inline void closeInjector6(void)  { closeInjector(6); }
static inline void openInjector7(void)   { openInjector(7); }
static inline void closeInjector7(void)  { closeInjector(7); }
static inline void openInjector8(void)   { openInjector(8); }
static inline void closeInjector8(void)  { closeInjector(8); }

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

static inline void injector1Toggle(void) { toggleInjector(1); }
static inline void injector2Toggle(void) { toggleInjector(2); }
static inline void injector3Toggle(void) { toggleInjector(3); }
static inline void injector4Toggle(void) { toggleInjector(4); }
static inline void injector5Toggle(void) { toggleInjector(5); }
static inline void injector6Toggle(void) { toggleInjector(6); }
static inline void injector7Toggle(void) { toggleInjector(7); }
static inline void injector8Toggle(void) { toggleInjector(8); }

// These are for Semi-Sequential and 6 Cylinder injection
//Standard 5 cylinder pairings
static inline void openInjector1and3(void) { openInjector1(); openInjector3(); }
static inline void closeInjector1and3(void) { closeInjector1(); closeInjector3(); }
static inline void openInjector2and4(void) { openInjector2(); openInjector4(); }
static inline void closeInjector2and4(void) { closeInjector2(); closeInjector4(); }
//Alternative output pairings
static inline void openInjector1and4(void) { openInjector1(); openInjector4(); }
static inline void closeInjector1and4(void) { closeInjector1(); closeInjector4(); }
static inline void openInjector2and3(void) { openInjector2(); openInjector3(); }
static inline void closeInjector2and3(void) { closeInjector2(); closeInjector3(); }

static inline void openInjector3and5(void) { openInjector3(); openInjector5(); }
static inline void closeInjector3and5(void) { closeInjector3(); closeInjector5(); }

static inline void openInjector2and5(void) { openInjector2(); openInjector5(); }
static inline void closeInjector2and5(void) { closeInjector2(); closeInjector5(); }
static inline void openInjector3and6(void) { openInjector3(); openInjector6(); }
static inline void closeInjector3and6(void) { closeInjector3(); closeInjector6(); }

static inline void openInjector1and5(void) { openInjector1(); openInjector5(); }
static inline void closeInjector1and5(void) { closeInjector1(); closeInjector5(); }
static inline void openInjector2and6(void) { openInjector2(); openInjector6(); }
static inline void closeInjector2and6(void) { closeInjector2(); closeInjector6(); }
static inline void openInjector3and7(void) { openInjector3(); openInjector7(); }
static inline void closeInjector3and7(void) { closeInjector3(); closeInjector7(); }
static inline void openInjector4and8(void) { openInjector4(); openInjector8(); }
static inline void closeInjector4and8(void) { closeInjector4(); closeInjector8(); }

// ================================= Ignition ================================= 

#if defined(OUTPUT_CONTROL_SUPPORTED)
extern ScheduleOutputControl ignitionOutputControl; //Specifies whether the coils are controlled directly (Via an IO pin) or using something like the MC33810
#endif

/**
 * @brief Register an ignition pin.
 * 
 * @param pin The pin number
 * @return ioPort 
 */
ioPort registerIgnitionPin(uint8_t pin);

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

static inline void beginCoil1Charge(void) { beginCoilCharge(1); }
static inline void endCoil1Charge(void) { endCoilCharge(1); }

static inline void beginCoil2Charge(void) { beginCoilCharge(2); }
static inline void endCoil2Charge(void) { endCoilCharge(2); }

static inline void beginCoil3Charge(void) { beginCoilCharge(3); }
static inline void endCoil3Charge(void) {endCoilCharge(3); }

static inline void beginCoil4Charge(void) { beginCoilCharge(4); }
static inline void endCoil4Charge(void) { endCoilCharge(4); }

static inline void beginCoil5Charge(void) { beginCoilCharge(5); }
static inline void endCoil5Charge(void) { endCoilCharge(5); }

static inline void beginCoil6Charge(void) { beginCoilCharge(6); }
static inline void endCoil6Charge(void) { endCoilCharge(6); }

static inline void beginCoil7Charge(void) { beginCoilCharge(7); }
static inline void endCoil7Charge(void) { endCoilCharge(7); }

static inline void beginCoil8Charge(void) { beginCoilCharge(8); }
static inline void endCoil8Charge(void) { endCoilCharge(8); }

//The below 4 calls are all part of the rotary ignition mode
static inline void beginTrailingCoilCharge(void) { beginCoil2Charge(); }
static inline void endTrailingCoilCharge1(void) { endCoil2Charge(); beginCoil3Charge(); } //Sets ign3 (Trailing select) high
static inline void endTrailingCoilCharge2(void) { endCoil2Charge(); endCoil3Charge(); } //sets ign3 (Trailing select) low

//As above but for ignition (Wasted COP mode)
static inline void beginCoil1and3Charge(void) { beginCoil1Charge(); beginCoil3Charge(); }
static inline void endCoil1and3Charge(void)   { endCoil1Charge();  endCoil3Charge(); }
static inline void beginCoil2and4Charge(void) { beginCoil2Charge(); beginCoil4Charge(); }
static inline void endCoil2and4Charge(void)   { endCoil2Charge();  endCoil4Charge(); }

//For 6cyl wasted COP mode)
static inline void beginCoil1and4Charge(void) { beginCoil1Charge(); beginCoil4Charge(); }
static inline void endCoil1and4Charge(void)   { endCoil1Charge();  endCoil4Charge(); }
static inline void beginCoil2and5Charge(void) { beginCoil2Charge(); beginCoil5Charge(); }
static inline void endCoil2and5Charge(void)   { endCoil2Charge();  endCoil5Charge(); }
static inline void beginCoil3and6Charge(void) { beginCoil3Charge(); beginCoil6Charge(); }
static inline void endCoil3and6Charge(void)   { endCoil3Charge(); endCoil6Charge(); }

//For 8cyl wasted COP mode)
static inline void beginCoil1and5Charge(void) { beginCoil1Charge(); beginCoil5Charge(); }
static inline void endCoil1and5Charge(void)   { endCoil1Charge();  endCoil5Charge(); }
static inline void beginCoil2and6Charge(void) { beginCoil2Charge(); beginCoil6Charge(); }
static inline void endCoil2and6Charge(void)   { endCoil2Charge();  endCoil6Charge(); }
static inline void beginCoil3and7Charge(void) { beginCoil3Charge(); beginCoil7Charge();  }
static inline void endCoil3and7Charge(void)   { endCoil3Charge(); endCoil7Charge(); }
static inline void beginCoil4and8Charge(void) { beginCoil4Charge(); beginCoil8Charge(); }
static inline void endCoil4and8Charge(void)   { endCoil4Charge();  endCoil8Charge(); }

#endif
