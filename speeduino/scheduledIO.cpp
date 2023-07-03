#include "scheduledIO.h"
#include "timers.h"

/** @file
 * Injector and Coil (toggle/open/close) control (under various situations, eg with particular cylinder count, rotary engine type or wasted spark ign, etc.).
 * Also accounts for presence of MC33810 injector/ignition (dwell, etc.) control circuit.
 * Functions here are typically assigned (at initialisation) to callback function variables (e.g. inj1StartFunction or inj1EndFunction) 
 * form where they are called (by scheduler.ino).
 */

#if defined(OUTPUT_CONTROL_SUPPORTED)
byte ignitionOutputControl; //Specifies whether the coils are controlled directly (Via an IO pin) or using something like the MC33810
#endif

#if defined(OUTPUT_CONTROL_SUPPORTED)
byte injectorOutputControl; /**< Specifies whether the injectors are controlled directly (Via an IO pin)
    or using something like the MC33810. 0 = Direct (OUTPUT_CONTROL_DIRECT), 10 = MC33810 (OUTPUT_CONTROL_MC33810) */
#endif

static inline ioPort registerIOPin(uint8_t pin, uint8_t ioType) {
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
