#include "globals.h"
#include "acc_mc33810.h"
#include "scheduledIO_direct_inj.h"

/** @file
 * Injector and Coil (toggle/open/close) control (under various situations, eg with particular cylinder count, rotary engine type or wasted spark ign, etc.).
 * Also accounts for presence of MC33810 injector/ignition (dwell, etc.) control circuit.
 * Functions here are typically assigned (at initialisation) to callback function variables (e.g. inj1StartFunction or inj1EndFunction) 
 * form where they are called (by scheduler.ino).
 */

 volatile byte injStatusMask = 0;

 /** @brief Injector open/close status bits */
char getInjectorStatus(void)
{
    return injStatusMask;
}

// LCOV_EXCL_START
// Exclude from code coverage, since this is all board output control

static void openInjector(uint8_t channel)
{
    if(injectorOutputControl != OUTPUT_CONTROL_MC33810) {
        openInjector_DIRECT(channel);
    } else {
        openInjector_MC33810(channel);
    };
    BIT_SET(injStatusMask, (channel)-1U);
}

static void closeInjector(uint8_t channel)
{
    if(injectorOutputControl != OUTPUT_CONTROL_MC33810) {
        closeInjector_DIRECT(channel);
    } else {
        closeInjector_MC33810(channel);
    };
    BIT_CLEAR(injStatusMask, (channel)-1U); 
}

void openInjector1(void)   { openInjector(1); }
void closeInjector1(void)  { closeInjector(1); }
void openInjector2(void)   { openInjector(2); }
void closeInjector2(void)  { closeInjector(2); }
void openInjector3(void)   { openInjector(3); }
void closeInjector3(void)  { closeInjector(3); }
void openInjector4(void)   { openInjector(4); }
void closeInjector4(void)  { closeInjector(4); }
void openInjector5(void)   { openInjector(5); }
void closeInjector5(void)  { closeInjector(5); }
void openInjector6(void)   { openInjector(6); }
void closeInjector6(void)  { closeInjector(6); }
void openInjector7(void)   { openInjector(7); }
void closeInjector7(void)  { closeInjector(7); }
void openInjector8(void)   { openInjector(8); }
void closeInjector8(void)  { closeInjector(8); }

// These are for Semi-Sequential and 5 Cylinder injection
//Standard 4 cylinder pairings
void openInjector1and3(void) { openInjector1(); openInjector3(); }
void closeInjector1and3(void) { closeInjector1(); closeInjector3(); }
void openInjector2and4(void) { openInjector2(); openInjector4(); }
void closeInjector2and4(void) { closeInjector2(); closeInjector4(); }
//Alternative output pairings
void openInjector1and4(void) { openInjector1(); openInjector4(); }
void closeInjector1and4(void) { closeInjector1(); closeInjector4(); }
void openInjector2and3(void) { openInjector2(); openInjector3(); }
void closeInjector2and3(void) { closeInjector2(); closeInjector3(); }

void openInjector3and5(void) { openInjector3(); openInjector5(); }
void closeInjector3and5(void) { closeInjector3(); closeInjector5(); }

void openInjector2and5(void) { openInjector2(); openInjector5(); }
void closeInjector2and5(void) { closeInjector2(); closeInjector5(); }
void openInjector3and6(void) { openInjector3(); openInjector6(); }
void closeInjector3and6(void) { closeInjector3(); closeInjector6(); }

void openInjector1and5(void) { openInjector1(); openInjector5(); }
void closeInjector1and5(void) { closeInjector1(); closeInjector5(); }
void openInjector2and6(void) { openInjector2(); openInjector6(); }
void closeInjector2and6(void) { closeInjector2(); closeInjector6(); }
void openInjector3and7(void) { openInjector3(); openInjector7(); }
void closeInjector3and7(void) { closeInjector3(); closeInjector7(); }
void openInjector4and8(void) { openInjector4(); openInjector8(); }
void closeInjector4and8(void) { closeInjector4(); closeInjector8(); }

// LCOV_EXCL_STOP