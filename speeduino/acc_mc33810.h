#ifndef MC33810_H
#define MC33810_H

#include <SPI.h>
#include "board_definition.h"

void initMC33810(uint8_t pinMC33810_1, uint8_t pinMC33810_2);

//These are default values for which injector is attached to which output on the IC. 
//They may (Probably will) be changed during init by the board specific config in init.ino
extern uint8_t MC33810_BIT_INJ[8];
extern uint8_t MC33810_BIT_IGN[8];

void openInjector1_MC33810(void);
void openInjector2_MC33810(void);
void openInjector3_MC33810(void);
void openInjector4_MC33810(void);
void openInjector5_MC33810(void);
void openInjector6_MC33810(void);
void openInjector7_MC33810(void);
void openInjector8_MC33810(void);

void closeInjector1_MC33810(void);
void closeInjector2_MC33810(void);
void closeInjector3_MC33810(void);
void closeInjector4_MC33810(void);
void closeInjector5_MC33810(void);
void closeInjector6_MC33810(void);
void closeInjector7_MC33810(void);
void closeInjector8_MC33810(void);

void coil1Charging_MC33810(void);
void coil1StopCharging_MC33810(void);
void coil2Charging_MC33810(void);
void coil2StopCharging_MC33810(void);
void coil3Charging_MC33810(void);
void coil3StopCharging_MC33810(void);
void coil4Charging_MC33810(void);
void coil4StopCharging_MC33810(void);
void coil5Charging_MC33810(void);
void coil5StopCharging_MC33810(void);
void coil6Charging_MC33810(void);
void coil6StopCharging_MC33810(void);
void coil7Charging_MC33810(void);
void coil7StopCharging_MC33810(void);
void coil8Charging_MC33810(void);
void coil8StopCharging_MC33810(void);


#endif