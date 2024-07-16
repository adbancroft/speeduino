#include "scheduledIO.h"

/** @file
 * Injector and Coil (toggle/open/close) control (under various situations, eg with particular cylinder count, rotary engine type or wasted spark ign, etc.).
 * Also accounts for presence of MC33810 injector/ignition (dwell, etc.) control circuit.
 * Functions here are typically assigned (at initialisation) to callback function variables (e.g. inj1StartFunction or inj1EndFunction) 
 * form where they are called (by scheduler.ino).
 */

static inline void nullAction(uint8_t) {
    // Do nothing
}

injector_control_t injectorControl = { nullAction, nullAction, nullAction };

void setInjectorControlActions(const injector_control_t &control) {
    extern injector_control_t injectorControl;
    injectorControl = control;
}

coil_control_t coilControl;// = { nullAction, nullAction };

void setIgnitionControlActions(const coil_control_t &control) {
    extern coil_control_t coilControl;
    coilControl = control;
}
