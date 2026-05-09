#include "globals.h"
#include "scheduledIO_direct_ign.h"
#include "acc_mc33810.h"
#include "timers.h"

// LCOV_EXCL_START
// Exclude from code coverage, since this is all board output control
 
static void tachoOutputOn(void) { if(configPage6.tachoMode) { tachoPulseLow(); } else { tachoOutputFlag = READY; } }
static void tachoOutputOff(void) { if(configPage6.tachoMode) { tachoPulseHigh(); } }

void beginCoil1Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilCharging_DIRECT(1); } else { coilCharging_MC33810(1); } tachoOutputOn(); }
void endCoil1Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilStopCharging_DIRECT(1); } else { coilStopCharging_MC33810(1); } tachoOutputOff(); }

void beginCoil2Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilCharging_DIRECT(2); } else { coilCharging_MC33810(2); } tachoOutputOn(); }
void endCoil2Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilStopCharging_DIRECT(2); } else { coilStopCharging_MC33810(2); } tachoOutputOff(); }

void beginCoil3Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilCharging_DIRECT(3); } else { coilCharging_MC33810(3); } tachoOutputOn(); }
void endCoil3Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilStopCharging_DIRECT(3); } else { coilStopCharging_MC33810(3); } tachoOutputOff(); }

void beginCoil4Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilCharging_DIRECT(4); } else { coilCharging_MC33810(4); } tachoOutputOn(); }
void endCoil4Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilStopCharging_DIRECT(4); } else { coilStopCharging_MC33810(4); } tachoOutputOff(); }

void beginCoil5Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilCharging_DIRECT(5); } else { coilCharging_MC33810(5); } tachoOutputOn(); }
void endCoil5Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilStopCharging_DIRECT(5); } else { coilStopCharging_MC33810(5); } tachoOutputOff(); }

void beginCoil6Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilCharging_DIRECT(6); } else { coilCharging_MC33810(6); } tachoOutputOn(); }
void endCoil6Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilStopCharging_DIRECT(6); } else { coilStopCharging_MC33810(6); } tachoOutputOff(); }

void beginCoil7Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilCharging_DIRECT(7); } else { coilCharging_MC33810(7); } tachoOutputOn(); }
void endCoil7Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilStopCharging_DIRECT(7); } else { coilStopCharging_MC33810(7); } tachoOutputOff(); }

void beginCoil8Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilCharging_DIRECT(8); } else { coilCharging_MC33810(8); } tachoOutputOn(); }
void endCoil8Charge(void) { if(ignitionOutputControl != OUTPUT_CONTROL_MC33810) { coilStopCharging_DIRECT(8); } else { coilStopCharging_MC33810(8); } tachoOutputOff(); }

//The below 3 calls are all part of the rotary ignition mode
void beginTrailingCoilCharge(void) { beginCoil2Charge(); }
void endTrailingCoilCharge1(void) { endCoil2Charge(); beginCoil3Charge(); } //Sets ign3 (Trailing select) high
void endTrailingCoilCharge2(void) { endCoil2Charge(); endCoil3Charge(); } //sets ign3 (Trailing select) low

//As above but for ignition (Wasted COP mode)
void beginCoil1and3Charge(void) { beginCoil1Charge(); beginCoil3Charge(); }
void endCoil1and3Charge(void)   { endCoil1Charge();  endCoil3Charge(); }
void beginCoil2and4Charge(void) { beginCoil2Charge(); beginCoil4Charge(); }
void endCoil2and4Charge(void)   { endCoil2Charge();  endCoil4Charge(); }

//For 6cyl wasted COP mode)
void beginCoil1and4Charge(void) { beginCoil1Charge(); beginCoil4Charge(); }
void endCoil1and4Charge(void)   { endCoil1Charge();  endCoil4Charge(); }
void beginCoil2and5Charge(void) { beginCoil2Charge(); beginCoil5Charge(); }
void endCoil2and5Charge(void)   { endCoil2Charge();  endCoil5Charge(); }
void beginCoil3and6Charge(void) { beginCoil3Charge(); beginCoil6Charge(); }
void endCoil3and6Charge(void)   { endCoil3Charge(); endCoil6Charge(); }

//For 8cyl wasted COP mode)
void beginCoil1and5Charge(void) { beginCoil1Charge(); beginCoil5Charge(); }
void endCoil1and5Charge(void)   { endCoil1Charge();  endCoil5Charge(); }
void beginCoil2and6Charge(void) { beginCoil2Charge(); beginCoil6Charge(); }
void endCoil2and6Charge(void)   { endCoil2Charge();  endCoil6Charge(); }
void beginCoil3and7Charge(void) { beginCoil3Charge(); beginCoil7Charge();  }
void endCoil3and7Charge(void)   { endCoil3Charge(); endCoil7Charge(); }
void beginCoil4and8Charge(void) { beginCoil4Charge(); beginCoil8Charge(); }
void endCoil4and8Charge(void)   { endCoil4Charge();  endCoil8Charge(); }

// LCOV_EXCL_STOP