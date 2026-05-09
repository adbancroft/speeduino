#include "scheduledIO_ign.h"
#include "scheduledIO_direct_ign.h"
#include "acc_mc33810.h"
#include "timers.h"
#include "globals.h"

static IgnIoControlMode _controlMode = IgnIoControlMode::Direct;

void initIgnIoControl(IgnIoControlMode controlMode)
{
    _controlMode = controlMode;
}

// LCOV_EXCL_START
// Exclude from code coverage, since this is all board output control
 
static void tachoOutputOn(void) { if(configPage6.tachoMode) { tachoPulseLow(); } else { tachoOutputFlag = READY; } }
static void tachoOutputOff(void) { if(configPage6.tachoMode) { tachoPulseHigh(); } }

static void beginCoilCharge(uint8_t channel) 
{ 
    if(_controlMode==IgnIoControlMode::Direct) 
    {
        coilCharging_DIRECT(channel);
    }
    else
    {
        coilCharging_MC33810(channel);
    }
    tachoOutputOn(); 
}

static void endCoilCharge(uint8_t channel)
{
    if(_controlMode==IgnIoControlMode::Direct) 
    {
        coilStopCharging_DIRECT(channel);
    }
    else
    {
        coilStopCharging_MC33810(channel);
    }
    tachoOutputOff();
}


void beginCoil1Charge(void) { beginCoilCharge(1U); }
void endCoil1Charge(void) { endCoilCharge(1U); }

void beginCoil2Charge(void) { beginCoilCharge(2U); }
void endCoil2Charge(void) { endCoilCharge(2U); }

void beginCoil3Charge(void) { beginCoilCharge(3U); }
void endCoil3Charge(void) { endCoilCharge(3U); }

void beginCoil4Charge(void) { beginCoilCharge(4U); }
void endCoil4Charge(void) { endCoilCharge(4U); }

void beginCoil5Charge(void) { beginCoilCharge(5U); }
void endCoil5Charge(void) { endCoilCharge(5U); }

void beginCoil6Charge(void) { beginCoilCharge(6U); }
void endCoil6Charge(void) { endCoilCharge(6U); }

void beginCoil7Charge(void) { beginCoilCharge(7U); }
void endCoil7Charge(void) { endCoilCharge(7U); }

void beginCoil8Charge(void) { beginCoilCharge(8U); }
void endCoil8Charge(void) { endCoilCharge(8U); }

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