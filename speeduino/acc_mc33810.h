#ifndef MC33810_H
#define MC33810_H

#include "board_definition.h"

#if defined(OUTPUT_CONTROL_SUPPORTED)

#include <SPI.h>
#include "port_pin.h"
#include "bit_manip.h"

struct mc33810_config_t {
    uint8_t pin;
    uint8_t injBits[4];
    uint8_t coilBits[4];
};
void initMC33810(const mc33810_config_t &ic1, const mc33810_config_t &ic2);

/// @cond
// Implementation details - ignore

// *one* MC33810
// 8 total channels: 4 injectors, 4 coils
// We expect 2 of these to be connected via 2 pins on the Speeduino board
struct mc33810_IC_t {
    ioPort pin;              ///< The Speeduino pin that connects to the MC33810
    volatile byte stateBits; ///< The MC33810 channel states, 1 per bit (on or off)
    mc33810_config_t config; ///< The config - needed for the bit indices
};

static inline void mc33810TransferState(mc33810_IC_t &mc33810) {
    static constexpr uint8_t MC33810_ONOFF_CMD = 0x30; //48 in decimal
    
    setPin_Low(mc33810.pin);
    (void)SPI.transfer16(word(MC33810_ONOFF_CMD, mc33810.stateBits));
    setPin_High(mc33810.pin);
}

static inline mc33810_IC_t& mc33810ICFromChannel(uint8_t channelIndex) {
    if (channelIndex<5) {
        // The lower 4 fuel & ignition channels are assigned to MC33810-1/pin1
        extern mc33810_IC_t mc33810_1;
        return mc33810_1;
    } else {
        // The upper 4 fuel & ignition channels are assigned to MC33810-2/pin2
        extern mc33810_IC_t mc33810_2;
        return mc33810_2;        
    }
}

///@endcond 

static inline void openInjector_MC33810(uint8_t channelIndex) {
    mc33810_IC_t& ic = mc33810ICFromChannel(channelIndex);
    BIT_SET(ic.stateBits, ic.config.injBits[(channelIndex-1U) % _countof(ic.config.injBits)]);

    mc33810TransferState(ic);
}

static inline void closeInjector_MC33810(uint8_t channelIndex) {
    mc33810_IC_t& ic = mc33810ICFromChannel(channelIndex);
    BIT_CLEAR(ic.stateBits, ic.config.injBits[(channelIndex-1U) % _countof(ic.config.injBits)]);

    mc33810TransferState(ic);
}

static inline void toggleInjector_MC33810(uint8_t channelIndex) {
    mc33810_IC_t& ic = mc33810ICFromChannel(channelIndex);
    BIT_TOGGLE(ic.stateBits, ic.config.injBits[(channelIndex-1U) % _countof(ic.config.injBits)]);

    mc33810TransferState(ic);
}

static inline void coilStartCharging_MC33810(uint8_t channelIndex) {
    mc33810_IC_t& ic = mc33810ICFromChannel(channelIndex);
    BIT_SET(ic.stateBits, ic.config.coilBits[(channelIndex-1U) % _countof(ic.config.coilBits)]);

    mc33810TransferState(ic);
}

static inline void coilStopCharging_MC33810(uint8_t channelIndex) {
    mc33810_IC_t& ic = mc33810ICFromChannel(channelIndex);
    BIT_CLEAR(ic.stateBits, ic.config.coilBits[(channelIndex-1U) % _countof(ic.config.coilBits)]);

    mc33810TransferState(ic);
}

#endif

#endif