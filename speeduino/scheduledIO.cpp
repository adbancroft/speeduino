#include "scheduledIO.h"
#include "pin_mapping.h"

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

static inline void registerPins(ioPort toRegister[], uint8_t toRegisterSize, const uint8_t pins[], ScheduleOutputControl ioType) {
    for (uint8_t index=0U; index<toRegisterSize; ++index) {
        toRegister[index] = registerIOPin(pins[index], ioType);
    }
}

void initialiseInjectorPins(const pin_mapping_t &pins) {
#if defined(OUTPUT_CONTROL_SUPPORTED)
    injectorOutputControl = isValidPin(pins.outputs.pinMC33810_1) ? OUTPUT_CONTROL_MC33810 : OUTPUT_CONTROL_DIRECT;
    registerPins(injectorPins, _countof(injectorPins), pins.outputs.pinInjectors, injectorOutputControl);
    if (injectorOutputControl==OUTPUT_CONTROL_MC33810) {
        initMC33810(pins);
    }
#else
    registerPins(injectorPins, _countof(injectorPins), pins.outputs.pinInjectors, OUTPUT_CONTROL_DIRECT);
#endif
}

void initialiseIgnitionPins(const pin_mapping_t &pins) {
#if defined(OUTPUT_CONTROL_SUPPORTED)
    ignitionOutputControl = isValidPin(pins.outputs.pinMC33810_2) ? OUTPUT_CONTROL_MC33810 : OUTPUT_CONTROL_DIRECT;
    registerPins(ignitionPins, _countof(ignitionPins), pins.outputs.pinCoils, ignitionOutputControl);
    if (ignitionOutputControl==OUTPUT_CONTROL_MC33810) {
        initMC33810(pins);
    }
#else
    registerPins(ignitionPins, _countof(ignitionPins), pins.outputs.pinCoils, OUTPUT_CONTROL_DIRECT);
#endif
}

