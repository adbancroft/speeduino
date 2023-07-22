/** @file
 * @brief Functions here are assigned (at initialisation) to callback function variables 
 * from where they are called (by scheduler.cpp).
 */

#pragma once

#include "scheduledIO.h"

static inline void nullCallback(void) { return; }

typedef void (*voidVoidCallback)(void);

// ================================= Injection ================================= 

static inline void openInjector1(void)   { openInjector(1); }
static inline void closeInjector1(void)  { closeInjector(1); }
#if INJ_CHANNELS>=2
static inline void openInjector2(void)   { openInjector(2); }
static inline void closeInjector2(void)  { closeInjector(2); }
#endif
#if INJ_CHANNELS>=3
static inline void openInjector3(void)   { openInjector(3); }
static inline void closeInjector3(void)  { closeInjector(3); }

static inline void openInjector1and3(void) { openInjector1(); openInjector3(); }
static inline void closeInjector1and3(void) { closeInjector1(); closeInjector3(); }
static inline void openInjector2and3(void) { openInjector2(); openInjector3(); }
static inline void closeInjector2and3(void) { closeInjector2(); closeInjector3(); }
#endif
#if INJ_CHANNELS>=4
static inline void openInjector4(void)   { openInjector(4); }
static inline void closeInjector4(void)  { closeInjector(4); }

static inline void openInjector1and4(void) { openInjector1(); openInjector4(); }
static inline void closeInjector1and4(void) { closeInjector1(); closeInjector4(); }
static inline void openInjector2and4(void) { openInjector2(); openInjector4(); }
static inline void closeInjector2and4(void) { closeInjector2(); closeInjector4(); }
#endif
#if INJ_CHANNELS>=5
static inline void openInjector5(void)   { openInjector(5); }
static inline void closeInjector5(void)  { closeInjector(5); }

static inline void openInjector1and5(void) { openInjector1(); openInjector5(); }
static inline void closeInjector1and5(void) { closeInjector1(); closeInjector5(); }
static inline void openInjector2and5(void) { openInjector2(); openInjector5(); }
static inline void closeInjector2and5(void) { closeInjector2(); closeInjector5(); }
static inline void openInjector3and5(void) { openInjector3(); openInjector5(); }
static inline void closeInjector3and5(void) { closeInjector3(); closeInjector5(); }
#endif
#if INJ_CHANNELS>=6
static inline void openInjector6(void)   { openInjector(6); }
static inline void closeInjector6(void)  { closeInjector(6); }

static inline void openInjector2and6(void) { openInjector2(); openInjector6(); }
static inline void closeInjector2and6(void) { closeInjector2(); closeInjector6(); }
static inline void openInjector3and6(void) { openInjector3(); openInjector6(); }
static inline void closeInjector3and6(void) { closeInjector3(); closeInjector6(); }
#endif
#if INJ_CHANNELS>=7
static inline void openInjector7(void)   { openInjector(7); }
static inline void closeInjector7(void)  { closeInjector(7); }

static inline void openInjector3and7(void) { openInjector3(); openInjector7(); }
static inline void closeInjector3and7(void) { closeInjector3(); closeInjector7(); }
#endif
#if INJ_CHANNELS>=8
static inline void openInjector8(void)   { openInjector(8); }
static inline void closeInjector8(void)  { closeInjector(8); }

static inline void openInjector4and8(void) { openInjector4(); openInjector8(); }
static inline void closeInjector4and8(void) { closeInjector4(); closeInjector8(); }
#endif

// ================================= Ignition ================================= 

static inline void beginCoil1Charge(void) { beginCoilCharge(1); }
static inline void endCoil1Charge(void) { endCoilCharge(1); }

#if IGN_CHANNELS>=2
static inline void beginCoil2Charge(void) { beginCoilCharge(2); }
static inline void endCoil2Charge(void) { endCoilCharge(2); }
#endif

#if IGN_CHANNELS>=3
static inline void beginCoil3Charge(void) { beginCoilCharge(3); }
static inline void endCoil3Charge(void) { endCoilCharge(3); }

// For wasted COP mode
static inline void beginCoil1and3Charge(void) { beginCoil1Charge(); beginCoil3Charge(); }
static inline void endCoil1and3Charge(void)   { endCoil1Charge();  endCoil3Charge(); }
#endif

#if IGN_CHANNELS>=4
static inline void beginCoil4Charge(void) { beginCoilCharge(4); }
static inline void endCoil4Charge(void) { endCoilCharge(4); }

// For wasted COP mode
static inline void beginCoil2and4Charge(void) { beginCoil2Charge(); beginCoil4Charge(); }
static inline void endCoil2and4Charge(void)   { endCoil2Charge();  endCoil4Charge(); }

static inline void beginCoil1and4Charge(void) { beginCoil1Charge(); beginCoil4Charge(); }
static inline void endCoil1and4Charge(void)   { endCoil1Charge();  endCoil4Charge(); }
#endif

#if IGN_CHANNELS>=5
static inline void beginCoil5Charge(void) { beginCoilCharge(5); }
static inline void endCoil5Charge(void) { endCoilCharge(5); }

static inline void beginCoil1and5Charge(void) { beginCoil1Charge(); beginCoil5Charge(); }
static inline void endCoil1and5Charge(void)   { endCoil1Charge();  endCoil5Charge(); }
static inline void beginCoil2and5Charge(void) { beginCoil2Charge(); beginCoil5Charge(); }
static inline void endCoil2and5Charge(void)   { endCoil2Charge();  endCoil5Charge(); }
#endif

#if IGN_CHANNELS>=6
static inline void beginCoil6Charge(void) { beginCoilCharge(6); }
static inline void endCoil6Charge(void) { endCoilCharge(6); }

static inline void beginCoil2and6Charge(void) { beginCoil2Charge(); beginCoil6Charge(); }
static inline void endCoil2and6Charge(void)   { endCoil2Charge();  endCoil6Charge(); }
static inline void beginCoil3and6Charge(void) { beginCoil3Charge(); beginCoil6Charge(); }
static inline void endCoil3and6Charge(void)   { endCoil3Charge(); endCoil6Charge(); }
#endif

#if IGN_CHANNELS>=7
static inline void beginCoil7Charge(void) { beginCoilCharge(7); }
static inline void endCoil7Charge(void) { endCoilCharge(7); }

static inline void beginCoil3and7Charge(void) { beginCoil3Charge(); beginCoil7Charge();  }
static inline void endCoil3and7Charge(void)   { endCoil3Charge(); endCoil7Charge(); }
#endif

#if IGN_CHANNELS>=8
static inline void beginCoil8Charge(void) { beginCoilCharge(8); }
static inline void endCoil8Charge(void) { endCoilCharge(8); }

static inline void beginCoil4and8Charge(void) { beginCoil4Charge(); beginCoil8Charge(); }
static inline void endCoil4and8Charge(void)   { endCoil4Charge();  endCoil8Charge(); }
#endif

//The below 3 calls are all part of the rotary ignition mode
static inline void beginTrailingCoilCharge(void) { beginCoil2Charge(); }
static inline void endTrailingCoilCharge1(void) { endCoil2Charge(); beginCoil3Charge(); } //Sets ign3 (Trailing select) high
static inline void endTrailingCoilCharge2(void) { endCoil2Charge(); endCoil3Charge(); } //sets ign3 (Trailing select) low
