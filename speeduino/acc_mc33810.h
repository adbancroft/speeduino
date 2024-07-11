#ifndef MC33810_H
#define MC33810_H

#include "board_definition.h"

#if defined(OUTPUT_CONTROL_SUPPORTED)

#include <SPI.h>
#include "port_pin.h"
#include "bit_manip.h"

extern byte pinMC33810_1_CS;
extern byte pinMC33810_2_CS;

void initMC33810(void);

/// @cond
// Implementation details - ignore

static inline void mc33810TransferState(ioPort pin, byte stateFlags) {
    static constexpr uint8_t MC33810_ONOFF_CMD = 0x30; //48 in decimal
    
    setPin_Low(pin);
    (void)SPI.transfer16(word(MC33810_ONOFF_CMD, stateFlags));
    setPin_High(pin);
}

static inline void mc33810TransferStateIC1(void) {
    extern ioPort portMC33810_1_CS;
    extern volatile uint8_t mc33810_1_requestedState;
    mc33810TransferState(portMC33810_1_CS, mc33810_1_requestedState);
}

static inline void setIC1StateBit(uint8_t bitIndex) {
    extern volatile uint8_t mc33810_1_requestedState;
    BIT_SET(mc33810_1_requestedState, bitIndex);
}

static inline void clearIC1StateBit(uint8_t bitIndex) {
    extern volatile uint8_t mc33810_1_requestedState;
    BIT_CLEAR(mc33810_1_requestedState, bitIndex);
}

static inline void toggleIC1StateBit(uint8_t bitIndex) {
    extern volatile uint8_t mc33810_1_requestedState;
    BIT_TOGGLE(mc33810_1_requestedState, bitIndex);
}

static inline void mc33810TransferStateIC2(void) {
    extern ioPort portMC33810_2_CS;
    extern volatile uint8_t mc33810_2_requestedState;
    mc33810TransferState(portMC33810_2_CS, mc33810_2_requestedState);
}

static inline void setIC2StateBit(uint8_t bitIndex) {
    extern volatile uint8_t mc33810_2_requestedState;
    BIT_SET(mc33810_2_requestedState, bitIndex);
}

static inline void clearIC2StateBit(uint8_t bitIndex) {
    extern volatile uint8_t mc33810_2_requestedState;
    BIT_CLEAR(mc33810_2_requestedState, bitIndex);
}

static inline void toggleIC2StateBit(uint8_t bitIndex) {
    extern volatile uint8_t mc33810_2_requestedState;
    BIT_TOGGLE(mc33810_2_requestedState, bitIndex);
}
///@endcond 

//These are default values for which injector is attached to which output on the IC. 
//They may (Probably will) be changed during init by the board specific config in init.ino
extern uint8_t MC33810_BIT_INJ1;
extern uint8_t MC33810_BIT_INJ2;
extern uint8_t MC33810_BIT_INJ3;
extern uint8_t MC33810_BIT_INJ4;
extern uint8_t MC33810_BIT_INJ5;
extern uint8_t MC33810_BIT_INJ6;
extern uint8_t MC33810_BIT_INJ7;
extern uint8_t MC33810_BIT_INJ8;

extern uint8_t MC33810_BIT_IGN1;
extern uint8_t MC33810_BIT_IGN2;
extern uint8_t MC33810_BIT_IGN3;
extern uint8_t MC33810_BIT_IGN4;
extern uint8_t MC33810_BIT_IGN5;
extern uint8_t MC33810_BIT_IGN6;
extern uint8_t MC33810_BIT_IGN7;
extern uint8_t MC33810_BIT_IGN8;

#define openInjector1_MC33810() { setIC1StateBit(MC33810_BIT_INJ1); mc33810TransferStateIC1(); }
#define openInjector2_MC33810() { setIC1StateBit(MC33810_BIT_INJ2); mc33810TransferStateIC1(); }
#define openInjector3_MC33810() { setIC1StateBit(MC33810_BIT_INJ3); mc33810TransferStateIC1(); }
#define openInjector4_MC33810() { setIC1StateBit(MC33810_BIT_INJ4); mc33810TransferStateIC1(); }
#define openInjector5_MC33810() { setIC2StateBit(MC33810_BIT_INJ5); mc33810TransferStateIC2(); }
#define openInjector6_MC33810() { setIC2StateBit(MC33810_BIT_INJ6); mc33810TransferStateIC2(); }
#define openInjector7_MC33810() { setIC2StateBit(MC33810_BIT_INJ7); mc33810TransferStateIC2(); }
#define openInjector8_MC33810() { setIC2StateBit(MC33810_BIT_INJ8); mc33810TransferStateIC2(); }

#define closeInjector1_MC33810() { clearIC1StateBit(MC33810_BIT_INJ1); mc33810TransferStateIC1(); }
#define closeInjector2_MC33810() { clearIC1StateBit(MC33810_BIT_INJ2); mc33810TransferStateIC1(); }
#define closeInjector3_MC33810() { clearIC1StateBit(MC33810_BIT_INJ3); mc33810TransferStateIC1(); }
#define closeInjector4_MC33810() { clearIC1StateBit(MC33810_BIT_INJ4); mc33810TransferStateIC1(); }
#define closeInjector5_MC33810() { clearIC2StateBit(MC33810_BIT_INJ5); mc33810TransferStateIC2(); }
#define closeInjector6_MC33810() { clearIC2StateBit(MC33810_BIT_INJ6); mc33810TransferStateIC2(); }
#define closeInjector7_MC33810() { clearIC2StateBit(MC33810_BIT_INJ7); mc33810TransferStateIC2(); }
#define closeInjector8_MC33810() { clearIC2StateBit(MC33810_BIT_INJ8); mc33810TransferStateIC2(); }

#define injector1Toggle_MC33810() { toggleIC1StateBit(MC33810_BIT_INJ1); mc33810TransferStateIC1(); }
#define injector2Toggle_MC33810() { toggleIC1StateBit(MC33810_BIT_INJ2); mc33810TransferStateIC1(); }
#define injector3Toggle_MC33810() { toggleIC1StateBit(MC33810_BIT_INJ3); mc33810TransferStateIC1(); }
#define injector4Toggle_MC33810() { toggleIC1StateBit(MC33810_BIT_INJ4); mc33810TransferStateIC1(); }
#define injector5Toggle_MC33810() { toggleIC2StateBit(MC33810_BIT_INJ5); mc33810TransferStateIC2(); }
#define injector6Toggle_MC33810() { toggleIC2StateBit(MC33810_BIT_INJ6); mc33810TransferStateIC2(); }
#define injector7Toggle_MC33810() { toggleIC2StateBit(MC33810_BIT_INJ7); mc33810TransferStateIC2(); }
#define injector8Toggle_MC33810() { toggleIC2StateBit(MC33810_BIT_INJ8); mc33810TransferStateIC2(); }

#define coil1High_MC33810() { setIC1StateBit(MC33810_BIT_IGN1); mc33810TransferStateIC1(); }
#define coil2High_MC33810() { setIC1StateBit(MC33810_BIT_IGN2); mc33810TransferStateIC1(); }
#define coil3High_MC33810() { setIC1StateBit(MC33810_BIT_IGN3); mc33810TransferStateIC1(); }
#define coil4High_MC33810() { setIC1StateBit(MC33810_BIT_IGN4); mc33810TransferStateIC1(); }
#define coil5High_MC33810() { setIC2StateBit(MC33810_BIT_IGN5); mc33810TransferStateIC2(); }
#define coil6High_MC33810() { setIC2StateBit(MC33810_BIT_IGN6); mc33810TransferStateIC2(); }
#define coil7High_MC33810() { setIC2StateBit(MC33810_BIT_IGN7); mc33810TransferStateIC2(); }
#define coil8High_MC33810() { setIC2StateBit(MC33810_BIT_IGN8); mc33810TransferStateIC2(); }

#define coil1Low_MC33810() { clearIC1StateBit(MC33810_BIT_IGN1); mc33810TransferStateIC1(); }
#define coil2Low_MC33810() { clearIC1StateBit(MC33810_BIT_IGN2); mc33810TransferStateIC1(); }
#define coil3Low_MC33810() { clearIC1StateBit(MC33810_BIT_IGN3); mc33810TransferStateIC1(); }
#define coil4Low_MC33810() { clearIC1StateBit(MC33810_BIT_IGN4); mc33810TransferStateIC1(); }
#define coil5Low_MC33810() { clearIC2StateBit(MC33810_BIT_IGN5); mc33810TransferStateIC2(); }
#define coil6Low_MC33810() { clearIC2StateBit(MC33810_BIT_IGN6); mc33810TransferStateIC2(); }
#define coil7Low_MC33810() { clearIC2StateBit(MC33810_BIT_IGN7); mc33810TransferStateIC2(); }
#define coil8Low_MC33810() { clearIC2StateBit(MC33810_BIT_IGN8); mc33810TransferStateIC2(); }

#endif

#endif