/*
These are some utility functions and variables used through the main code
*/
#ifndef UTILS_H
#define UTILS_H

#include "globals.h"
#include "pin_mapping.h"

#define COMPARATOR_EQUAL 0
#define COMPARATOR_NOT_EQUAL 1
#define COMPARATOR_GREATER 2
#define COMPARATOR_GREATER_EQUAL 3
#define COMPARATOR_LESS 4
#define COMPARATOR_LESS_EQUAL 5
#define COMPARATOR_AND 6
#define COMPARATOR_XOR 7

#define BITWISE_DISABLED 0
#define BITWISE_AND 1
#define BITWISE_OR 2
#define BITWISE_XOR 3

#define REUSE_RULES 240

void initialiseResetControl(const pin_mapping_t &pins);
void setResetControlPinState(void);
void resetPrevent(void);
void resetAllow(void);
static inline bool isResetControlEnabled(void) {
  return configPage4.resetControlConfig != RESET_CONTROL_DISABLED;
}

void initialiseIgnitionByPass(const pin_mapping_t &pins);
void ignitionByPassOn(void);
void ignitionByPassOff(void);
static inline bool isIgnBypassEnabled(void) {
  return configPage4.ignBypassEnabled != 0U;
}

void initialiseProgrammableIO(const pin_mapping_t &pins);
void checkProgrammableIO(void);
int16_t ProgrammableIOGetData(uint16_t index);

void initialiseLaunchControl(const pin_mapping_t &pins);
bool isClutchTriggerOn(void);
static inline bool isLaunchEnabled(void) {
  return configPage6.launchEnabled != 0U;
}
static inline bool isFlatShiftEnabled(void) {
  return configPage6.flatSEnable != 0U;
}
static inline bool isClutchTriggerEnabled(void) {
  return isLaunchEnabled() || isFlatShiftEnabled();
}

#endif // UTILS_H
