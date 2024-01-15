#pragma once

/**
 * @file
 * 
 * @brief Decoder trigger types. This file is used to limit header dependecies.
 * 
 */

#include <stdint.h>
#include "board_selector.h"

/** @brief A handler function for a trigger. 
 * 
 * This is an ISR & will be called when the corresponding interrupt is triggered.
 * See Arduino <code>attachInterrupt</code> docs.
 */
using trigger_handler_t = void (*)(void);

/** @brief Defines when the interrupt should be triggered
 * 
 * Options are:
 * * CHANGE to trigger the interrupt whenever the pin changes value
 * * RISING to trigger when the pin goes from low to high,
 * * FALLING for when the pin goes from high to low.
 */
#if defined(CORE_SAMD21)
using trigger_mode_t = PinStatus;
#else
using trigger_mode_t = uint8_t;
#endif

/** @brief Encapsulates a decoder "trigger". I.e. what is called & when. */
struct trigger_t {
  trigger_handler_t handler;
  trigger_mode_t mode;
};

/**
 * @brief Test is a trigger is valid. Not all decoders implement all 3 triggers.
 * 
 * @param trigger Trigger to test
 * @return true If valid
 * @return false If invalid & should not be called/applied.
 */
static inline bool isValid(const trigger_t &trigger) {
  return trigger.handler!=nullptr;
}

/** @brief An empty trigger definition - for consistency
 * 
 * Note that this is a compile time only symbol & has zero runtime overhead. 
 */
static constexpr trigger_t NULL_TRIGGER = { nullptr, 0U };

/** @brief The external API for a decoder. */
struct decoder_t {
    uint16_t (*getRPM)(void); ///< [Required] Returns the current RPM, as calculated by the decoder.
    int (*getCrankAngle)(void); ///< [Required] Returns the current crank angle, as calculated by the decoder.
    
    /** @brief [Optional] Called as part of the ignition timing calculations to allow the decoder to adjust timing.
     * 
     * If per-tooth timing is enabled, the decoder can adjust ignition timing. Typically this is done in by tracking
     * individual teeth, which is more accurate than using crank angle alone.
     */
    void (*triggerSetEndTeeth)(void); 

    trigger_t primaryTrigger; ///< The decoder primary trigger, typically the crank.
    trigger_t secondaryTrigger; ///< The decoder secondary trigger, typically a cam.
    trigger_t tertiaryTrigger; ///< The decoder 3rd trigger, maybe a cam or VVT
};
