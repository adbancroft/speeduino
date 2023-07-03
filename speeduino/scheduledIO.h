/** @file
 * Injector and Coil (toggle/open/close) control (under various situations, eg with particular cylinder count, rotary engine type or wasted spark ign, etc.).
 * Also accounts for presence of MC33810 injector/ignition (dwell, etc.) control circuit.
 * Functions here are typically assigned (at initialisation) to callback function variables (e.g. inj1StartFunction or inj1EndFunction) 
 * form where they are called (by scheduler.ino).
 */

#ifndef SCHEDULEDIO_H
#define SCHEDULEDIO_H

#include "board_definition.h"
#include "timers.h"
#include "acc_mc33810.h"
#include "scheduledIO_direct.h"

static inline void nullCallback(void) { return; }

typedef void (*voidVoidCallback)(void);

// ================================= Injection ================================= 

/**
 * @brief Register an injector pin.
 * 
 * @param pin The pin number
 * @return ioPort 
 */
ioPort registerInjectorPin(uint8_t pin);

static inline void tachoOutputOn(void) { if(configPage6.tachoMode) { TACHO_PULSE_LOW(); } else { tachoOutputFlag = READY; } }
static inline void tachoOutputOff(void) { if(configPage6.tachoMode) { TACHO_PULSE_HIGH(); } }

#if defined(OUTPUT_CONTROL_SUPPORTED)
extern byte injectorOutputControl; /**< Specifies whether the injectors are controlled directly (Via an IO pin)
    or using something like the MC33810. 0 = Direct (OUTPUT_CONTROL_DIRECT), 10 = MC33810 (OUTPUT_CONTROL_MC33810) */

static inline void openInjector1(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector1_DIRECT(); }   else { openInjector1_MC33810(); } }
static inline void closeInjector1(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector1_DIRECT(); }  else { closeInjector1_MC33810(); } }
static inline void openInjector2(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector2_DIRECT(); }   else { openInjector2_MC33810(); } }
static inline void closeInjector2(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector2_DIRECT(); }  else { closeInjector2_MC33810(); } }
static inline void openInjector3(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector3_DIRECT(); }   else { openInjector3_MC33810(); } }
static inline void closeInjector3(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector3_DIRECT(); }  else { closeInjector3_MC33810(); } }
static inline void openInjector4(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector4_DIRECT(); }   else { openInjector4_MC33810(); } }
static inline void closeInjector4(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector4_DIRECT(); }  else { closeInjector4_MC33810(); } }
static inline void openInjector5(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector5_DIRECT(); }   else { openInjector5_MC33810(); } }
static inline void closeInjector5(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector5_DIRECT(); }  else { closeInjector5_MC33810(); } }
static inline void openInjector6(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector6_DIRECT(); }   else { openInjector6_MC33810(); } }
static inline void closeInjector6(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector6_DIRECT(); }  else { closeInjector6_MC33810(); } }
static inline void openInjector7(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector7_DIRECT(); }   else { openInjector7_MC33810(); } }
static inline void closeInjector7(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector7_DIRECT(); }  else { closeInjector7_MC33810(); } }
static inline void openInjector8(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector8_DIRECT(); }   else { openInjector8_MC33810(); } }
static inline void closeInjector8(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector8_DIRECT(); }  else { closeInjector8_MC33810(); } }
#else
static inline void openInjector1(void)   { openInjector1_DIRECT(); }
static inline void closeInjector1(void)  { closeInjector1_DIRECT(); }
static inline void openInjector2(void)   { openInjector2_DIRECT(); }
static inline void closeInjector2(void)  { closeInjector2_DIRECT(); }
static inline void openInjector3(void)   { openInjector3_DIRECT(); }
static inline void closeInjector3(void)  { closeInjector3_DIRECT(); }
static inline void openInjector4(void)   { openInjector4_DIRECT(); }
static inline void closeInjector4(void)  { closeInjector4_DIRECT(); }
static inline void openInjector5(void)   { openInjector5_DIRECT(); }
static inline void closeInjector5(void)  { closeInjector5_DIRECT(); }
static inline void openInjector6(void)   { openInjector6_DIRECT(); }
static inline void closeInjector6(void)  { closeInjector6_DIRECT(); }
static inline void openInjector7(void)   { openInjector7_DIRECT(); }
static inline void closeInjector7(void)  { closeInjector7_DIRECT(); }
static inline void openInjector8(void)   { openInjector8_DIRECT(); }
static inline void closeInjector8(void)  { closeInjector8_DIRECT(); }
#endif

#if defined(OUTPUT_CONTROL_SUPPORTED)
static inline void injector1Toggle(void) { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { injector1Toggle_DIRECT(); } else { injector1Toggle_MC33810(); } }
static inline void injector2Toggle(void) { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { injector2Toggle_DIRECT(); } else { injector2Toggle_MC33810(); } }
static inline void injector3Toggle(void) { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { injector3Toggle_DIRECT(); } else { injector3Toggle_MC33810(); } }
static inline void injector4Toggle(void) { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { injector4Toggle_DIRECT(); } else { injector4Toggle_MC33810(); } }
static inline void injector5Toggle(void) { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { injector5Toggle_DIRECT(); } else { injector5Toggle_MC33810(); } }
static inline void injector6Toggle(void) { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { injector6Toggle_DIRECT(); } else { injector6Toggle_MC33810(); } }
static inline void injector7Toggle(void) { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { injector7Toggle_DIRECT(); } else { injector7Toggle_MC33810(); } }
static inline void injector8Toggle(void) { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { injector8Toggle_DIRECT(); } else { injector8Toggle_MC33810(); } }
#else
static inline void injector1Toggle(void) { injector1Toggle_DIRECT(); }
static inline void injector2Toggle(void) { injector2Toggle_DIRECT(); }
static inline void injector3Toggle(void) { injector3Toggle_DIRECT(); }
static inline void injector4Toggle(void) { injector4Toggle_DIRECT(); }
static inline void injector5Toggle(void) { injector5Toggle_DIRECT(); }
static inline void injector6Toggle(void) { injector6Toggle_DIRECT(); }
static inline void injector7Toggle(void) { injector7Toggle_DIRECT(); }
static inline void injector8Toggle(void) { injector8Toggle_DIRECT(); }
#endif

// These are for Semi-Sequential and 5 Cylinder injection
//Standard 4 cylinder pairings
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

/**
 * @brief Register an ignition pin.
 * 
 * @param pin The pin number
 * @return ioPort 
 */
ioPort registerIgnitionPin(uint8_t pin);


#if defined(OUTPUT_CONTROL_SUPPORTED)
extern byte ignitionOutputControl; //Specifies whether the coils are controlled directly (Via an IO pin) or using something like the MC33810
static inline void beginCoil1Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil1Charging_DIRECT(); } else { coil1Charging_MC33810(); } tachoOutputOn(); }
static inline void endCoil1Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil1StopCharging_DIRECT(); } else { coil1StopCharging_MC33810(); } tachoOutputOff(); }

static inline void beginCoil2Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil2Charging_DIRECT(); } else { coil2Charging_MC33810(); } tachoOutputOn(); }
static inline void endCoil2Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil2StopCharging_DIRECT(); } else { coil2StopCharging_MC33810(); } tachoOutputOff(); }

static inline void beginCoil3Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil3Charging_DIRECT(); } else { coil3Charging_MC33810(); } tachoOutputOn(); }
static inline void endCoil3Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil3StopCharging_DIRECT(); } else { coil3StopCharging_MC33810(); } tachoOutputOff(); }

static inline void beginCoil4Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil4Charging_DIRECT(); } else { coil4Charging_MC33810(); } tachoOutputOn(); }
static inline void endCoil4Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil4StopCharging_DIRECT(); } else { coil4StopCharging_MC33810(); } tachoOutputOff(); }

static inline void beginCoil5Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil5Charging_DIRECT(); } else { coil5Charging_MC33810(); } tachoOutputOn(); }
static inline void endCoil5Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil5StopCharging_DIRECT(); } else { coil5StopCharging_MC33810(); } tachoOutputOff(); }

static inline void beginCoil6Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil6Charging_DIRECT(); } else { coil6Charging_MC33810(); } tachoOutputOn(); }
static inline void endCoil6Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil6StopCharging_DIRECT(); } else { coil6StopCharging_MC33810(); } tachoOutputOff(); }

static inline void beginCoil7Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil7Charging_DIRECT(); } else { coil7Charging_MC33810(); } tachoOutputOn(); }
static inline void endCoil7Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil7StopCharging_DIRECT(); } else { coil7StopCharging_MC33810(); } tachoOutputOff(); }

static inline void beginCoil8Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil8Charging_DIRECT(); } else { coil8Charging_MC33810(); } tachoOutputOn(); }
static inline void endCoil8Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coil8StopCharging_DIRECT(); } else { coil8StopCharging_MC33810(); } tachoOutputOff(); }
#else
static inline void beginCoil1Charge(void) { coil1Charging_DIRECT(); tachoOutputOn(); }
static inline void endCoil1Charge(void) { coil1StopCharging_DIRECT(); tachoOutputOff(); }

static inline void beginCoil2Charge(void) { coil2Charging_DIRECT(); tachoOutputOn(); }
static inline void endCoil2Charge(void) { coil2StopCharging_DIRECT(); tachoOutputOff(); }

static inline void beginCoil3Charge(void) { coil3Charging_DIRECT(); tachoOutputOn(); }
static inline void endCoil3Charge(void) { coil3StopCharging_DIRECT(); tachoOutputOff(); }

static inline void beginCoil4Charge(void) { coil4Charging_DIRECT(); tachoOutputOn(); }
static inline void endCoil4Charge(void) { coil4StopCharging_DIRECT(); tachoOutputOff(); }

static inline void beginCoil5Charge(void) { coil5Charging_DIRECT(); tachoOutputOn(); }
static inline void endCoil5Charge(void) { coil5StopCharging_DIRECT(); tachoOutputOff(); }

static inline void beginCoil6Charge(void) { coil6Charging_DIRECT(); tachoOutputOn(); }
static inline void endCoil6Charge(void) { coil6StopCharging_DIRECT(); tachoOutputOff(); }

static inline void beginCoil7Charge(void) { coil7Charging_DIRECT(); tachoOutputOn(); }
static inline void endCoil7Charge(void) { coil7StopCharging_DIRECT(); tachoOutputOff(); }

static inline void beginCoil8Charge(void) { coil8Charging_DIRECT(); tachoOutputOn(); }
static inline void endCoil8Charge(void) { coil8StopCharging_DIRECT(); tachoOutputOff(); }
#endif

//The below 3 calls are all part of the rotary ignition mode
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
