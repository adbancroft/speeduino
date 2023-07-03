#pragma once

#include "globals.h"
#include "port_pin.h"

//These are for the direct port manipulation of the injectors, coils
extern ioPort injectorPins[INJ_CHANNELS];
extern ioPort ignitionPins[IGN_CHANNELS];

//Macros are used to define how each injector control system functions. These are then called by the master openInjectx() function.
//The DIRECT macros (ie individual pins) are defined below. Others should be defined in their relevant acc_x.h file
#define openInjector1_DIRECT()  { setPin_High(injectorPins[0]); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ1); }
#define closeInjector1_DIRECT() { setPin_Low(injectorPins[0]);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ1); }
#define openInjector2_DIRECT()  { setPin_High(injectorPins[1]); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ2); }
#define closeInjector2_DIRECT() { setPin_Low(injectorPins[1]);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ2); }
#define openInjector3_DIRECT()  { setPin_High(injectorPins[2]); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ3); }
#define closeInjector3_DIRECT() { setPin_Low(injectorPins[2]);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ3); }
#define openInjector4_DIRECT()  { setPin_High(injectorPins[3]); BIT_SET(currentStatus.status1, BIT_STATUS1_INJ4); }
#define closeInjector4_DIRECT() { setPin_Low(injectorPins[3]);  BIT_CLEAR(currentStatus.status1, BIT_STATUS1_INJ4); }
#define openInjector5_DIRECT()  { setPin_High(injectorPins[4]); }
#define closeInjector5_DIRECT() { setPin_Low(injectorPins[4]); }
#define openInjector6_DIRECT()  { setPin_High(injectorPins[5]); }
#define closeInjector6_DIRECT() { setPin_Low(injectorPins[5]); }
#define openInjector7_DIRECT()  { setPin_High(injectorPins[6]); }
#define closeInjector7_DIRECT() { setPin_Low(injectorPins[6]); }
#define openInjector8_DIRECT()  { setPin_High(injectorPins[7]); }
#define closeInjector8_DIRECT() { setPin_Low(injectorPins[7]); }

#define coil1Low_DIRECT()       setPin_High(ignitionPins[0])
#define coil1High_DIRECT()      setPin_Low(ignitionPins[0])
#define coil2Low_DIRECT()       setPin_High(ignitionPins[1])
#define coil2High_DIRECT()      setPin_Low(ignitionPins[1])
#define coil3Low_DIRECT()       setPin_High(ignitionPins[2])
#define coil3High_DIRECT()      setPin_Low(ignitionPins[2])
#define coil4Low_DIRECT()       setPin_High(ignitionPins[3])
#define coil4High_DIRECT()      setPin_Low(ignitionPins[3])
#define coil5Low_DIRECT()       setPin_High(ignitionPins[4])
#define coil5High_DIRECT()      setPin_Low(ignitionPins[4])
#define coil6Low_DIRECT()       setPin_High(ignitionPins[5])
#define coil6High_DIRECT()      setPin_Low(ignitionPins[5])
#define coil7Low_DIRECT()       setPin_High(ignitionPins[6])
#define coil7High_DIRECT()      setPin_Low(ignitionPins[6])
#define coil8Low_DIRECT()       setPin_High(ignitionPins[7])
#define coil8High_DIRECT()      setPin_Low(ignitionPins[7])

//Set the value of the coil pins to the coilHIGH or coilLOW state
#define coil1Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil1Low_DIRECT() : coil1High_DIRECT())
#define coil1StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil1High_DIRECT() : coil1Low_DIRECT())
#define coil2Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil2Low_DIRECT() : coil2High_DIRECT())
#define coil2StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil2High_DIRECT() : coil2Low_DIRECT())
#define coil3Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil3Low_DIRECT() : coil3High_DIRECT())
#define coil3StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil3High_DIRECT() : coil3Low_DIRECT())
#define coil4Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil4Low_DIRECT() : coil4High_DIRECT())
#define coil4StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil4High_DIRECT() : coil4Low_DIRECT())
#define coil5Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil5Low_DIRECT() : coil5High_DIRECT())
#define coil5StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil5High_DIRECT() : coil5Low_DIRECT())
#define coil6Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil6Low_DIRECT() : coil6High_DIRECT())
#define coil6StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil6High_DIRECT() : coil6Low_DIRECT())
#define coil7Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil7Low_DIRECT() : coil7High_DIRECT())
#define coil7StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil7High_DIRECT() : coil7Low_DIRECT())
#define coil8Charging_DIRECT()      (configPage4.IgInv == GOING_HIGH ? coil8Low_DIRECT() : coil8High_DIRECT())
#define coil8StopCharging_DIRECT()  (configPage4.IgInv == GOING_HIGH ? coil8High_DIRECT() : coil8Low_DIRECT())

#define coil1Toggle_DIRECT() { togglePin(ignitionPins[0]); }
#define coil2Toggle_DIRECT() { togglePin(ignitionPins[1]); }
#define coil3Toggle_DIRECT() { togglePin(ignitionPins[2]); }
#define coil4Toggle_DIRECT() { togglePin(ignitionPins[3]); }
#define coil5Toggle_DIRECT() { togglePin(ignitionPins[4]); }
#define coil6Toggle_DIRECT() { togglePin(ignitionPins[5]); }
#define coil7Toggle_DIRECT() { togglePin(ignitionPins[6]); }
#define coil8Toggle_DIRECT() { togglePin(ignitionPins[7]); }

#define injector1Toggle_DIRECT() { togglePin(injectorPins[0]); }
#define injector2Toggle_DIRECT() { togglePin(injectorPins[1]); }
#define injector3Toggle_DIRECT() { togglePin(injectorPins[2]); }
#define injector4Toggle_DIRECT() { togglePin(injectorPins[3]); }
#define injector5Toggle_DIRECT() { togglePin(injectorPins[4]); }
#define injector6Toggle_DIRECT() { togglePin(injectorPins[5]); }
#define injector7Toggle_DIRECT() { togglePin(injectorPins[6]); }
#define injector8Toggle_DIRECT() { togglePin(injectorPins[7]); }
