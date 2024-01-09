#pragma once

#include <stdint.h>
#include "board_selector.h"

using trigger_handler_t = void (*)(void);
#if defined(CORE_SAMD21)
using trigger_edge_t = PinStatus;
#else
using trigger_edge_t = uint8_t;
#endif

struct trigger_t {
  trigger_handler_t handler;
  trigger_edge_t edge; // CHANGE, RISING, FALLING
};

static inline bool isValid(const trigger_t &trigger) {
  return trigger.handler!=nullptr;
}

struct decoder_t {
    uint16_t (*getRPM)(void); //Pointer to the getRPM function (Gets pointed to the relevant decoder)
    int (*getCrankAngle)(void); //Pointer to the getCrank Angle function (Gets pointed to the relevant decoder)
    void (*triggerSetEndTeeth)(void); //Pointer to the triggerSetEndTeeth function of each decoder

    trigger_t primaryTrigger;
    trigger_t secondaryTrigger;
    trigger_t tertiaryTrigger;
};

extern decoder_t decoder;

/** @brief Initialize the decoder as used by the tune. */
void initialiseDecoder(void);

void attachPrimaryInterrupt(const trigger_t &trigger);
void attachSecondaryInterrupt(const trigger_t &trigger);
void attachTertiaryInterrupt(const trigger_t &trigger);

uint8_t getTriggerPinState(const trigger_t &trigger);

// Old API - here for simplicity
static inline int getCrankAngle(void) { return decoder.getCrankAngle(); }
static inline uint16_t getRPM(void) { return decoder.getRPM(); }
static inline void triggerSetEndTeeth(void) { decoder.triggerSetEndTeeth(); }
static inline uint8_t READ_PRI_TRIGGER(void) { return getTriggerPinState(decoder.primaryTrigger); }
static inline uint8_t READ_SEC_TRIGGER(void) { return getTriggerPinState(decoder.secondaryTrigger); }
static inline uint8_t READ_THIRD_TRIGGER(void) { return getTriggerPinState(decoder.tertiaryTrigger); }
