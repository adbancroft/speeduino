#ifndef IDLE_H
#define IDLE_H

#include "globals.h"
#include "table2d.h"
#include "board_definition.h"
#include "port_pin.h"
#include "pin_mapping.h"

#define IAC_ALGORITHM_NONE    0U
#define IAC_ALGORITHM_ONOFF   1U
#define IAC_ALGORITHM_PWM_OL  2U
#define IAC_ALGORITHM_PWM_CL  3U
#define IAC_ALGORITHM_STEP_OL 4U
#define IAC_ALGORITHM_STEP_CL 5U
#define IAC_ALGORITHM_PWM_OLCL  6U //Openloop plus closedloop IAC control
#define IAC_ALGORITHM_STEP_OLCL  7U //Openloop plus closedloop IAC control

#define STEPPER_FORWARD 0
#define STEPPER_BACKWARD 1
#define STEPPER_POWER_WHEN_ACTIVE 0
#define IDLE_TABLE_SIZE 10

enum StepperStatus {SOFF, STEPPING, COOLING}; //The 2 statuses that a stepper can have. STEPPING means that a high pulse is currently being sent and will need to be turned off at some point.

struct StepperIdle
{
  int curIdleStep; //Tracks the current location of the stepper
  int targetIdleStep; //What the targeted step is
  volatile StepperStatus stepperStatus;
  volatile unsigned long stepStartTime;
};

void initialiseIdle(bool forcehoming, const pin_mapping_t &pins);
void initialiseIdle(bool forcehoming);
void idleControl(void);
void initialiseIdleUpOutput(void);
void disableIdle(void);
void idleInterrupt(void);

static inline bool isIdlePwm(void) {
    return (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) 
    || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL)
    || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OLCL)
    ;
}
static inline bool isIdleStepper(void) {
    return (configPage6.iacAlgorithm==IAC_ALGORITHM_STEP_OL) 
    || (configPage6.iacAlgorithm ==IAC_ALGORITHM_STEP_CL) 
    || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OLCL)
    ;
}

static inline bool isIdleUpEnabled(void) {
  return configPage2.idleUpEnabled != 0U;
}
static inline bool isIdleUpOutputEnabled(void) {
  return isIdleUpEnabled() && configPage2.idleUpOutputEnabled!=0U;
}
static inline bool isIdle2PwmEnabled(void) {
  return isIdlePwm() && (configPage6.iacChannels==1U);
}
#endif
