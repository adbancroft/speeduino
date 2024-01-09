#pragma once

#include "decoder_trigger_types.h"
#include "pin_mapping.h"

extern decoder_t decoder;

pin_mapping_t initialiseDecoder(uint8_t pattern, pin_mapping_t pins);
void reInitialiseDecoder(void);

void attachPrimaryInterrupt(const trigger_t &trigger);
void attachSecondaryInterrupt(const trigger_t &trigger);
void attachTertiaryInterrupt(const trigger_t &trigger);

uint8_t getTriggerPinState(const trigger_t &trigger);

// Old API - here for simplicity
static inline int getCrankAngle(void) { return decoder.getCrankAngle(); }
static inline uint16_t getRPM(void) { return decoder.getRPM(); }
static inline uint8_t READ_PRI_TRIGGER(void) { return getTriggerPinState(decoder.primaryTrigger); }
static inline uint8_t READ_SEC_TRIGGER(void) { return getTriggerPinState(decoder.secondaryTrigger); }
static inline uint8_t READ_THIRD_TRIGGER(void) { return getTriggerPinState(decoder.tertiaryTrigger); }
