#include "scheduledIO_direct.h"

static inline void registerPins(ioPort toRegister[], uint8_t toRegisterSize, const uint8_t pins[]) {
    for (uint8_t index=0U; index<toRegisterSize; ++index) {
        toRegister[index] = pinToOutputPort(pins[index]);
    }
}

// cppcheck-suppress misra-c2012-8.4
ioPort injectorPins[INJ_CHANNELS];

void initialiseInjectorPins_DIRECT(const uint8_t pins[]) {
    registerPins(injectorPins, _countof(injectorPins), pins);
}

// cppcheck-suppress misra-c2012-8.4
ioPort ignitionPins[IGN_CHANNELS];

void initialiseIgnitionPins_DIRECT(const uint8_t pins[]) {
    registerPins(ignitionPins, _countof(ignitionPins), pins);
}
