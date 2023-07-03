#include "scheduledIO.h"

/** @file
 * Injector and Coil (toggle/open/close) control (under various situations, eg with particular cylinder count, rotary engine type or wasted spark ign, etc.).
 * Also accounts for presence of MC33810 injector/ignition (dwell, etc.) control circuit.
 * Functions here are typically assigned (at initialisation) to callback function variables (e.g. inj1StartFunction or inj1EndFunction) 
 * form where they are called (by scheduler.ino).
 */

#if defined(OUTPUT_CONTROL_SUPPORTED)
ScheduleOutputControl ignitionOutputControl;
#endif

#if defined(OUTPUT_CONTROL_SUPPORTED)
ScheduleOutputControl injectorOutputControl;
#endif

static inline ioPort registerIOPin(uint8_t pin, ScheduleOutputControl ioType) {
    if(ioType == OUTPUT_CONTROL_DIRECT) {
        return pinToOutputPort(pin);
    } else {
        return nullIoPort();
    }
}

ioPort registerInjectorPin(uint8_t pin) {
#if defined(OUTPUT_CONTROL_SUPPORTED)
    return registerIOPin(pin, injectorOutputControl);
#else
    return registerIOPin(pin, OUTPUT_CONTROL_DIRECT);
#endif
}

ioPort registerIgnitionPin(uint8_t pin) {
#if defined(OUTPUT_CONTROL_SUPPORTED)
    return registerIOPin(pin, ignitionOutputControl);
#else
    return registerIOPin(pin, OUTPUT_CONTROL_DIRECT);
#endif
}
