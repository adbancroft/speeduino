#pragma once

#include <stdint.h>
#include "board_definition.h"
#include "pre_processor.h"

/** @brief A flag value that indicates a variable doesn't reference any pin. */
#if !defined(NOT_A_PIN)
#define NOT_A_PIN 0
#endif

/**
 * @brief Test if the pin number is valid
 * 
 * @param pin Pin number to test
 */
static inline bool isValidPin(uint8_t pin) {
    // Zero is not a valid pin on any board (it's usually the GP IO pin)
    // In INI, it's used to flag that the board default should be used.
    return pin>0U && pin<UINT8_C(BOARD_MAX_IO_PINS);
}

/** @brief Allows direct manipulation of a pin via bit twiddling 
 * 
 * Much faster than digitalRead or digitalWrite.
 * @see https://docs.arduino.cc/hacking/software/PortManipulation
 */
struct ioPort {
    /** @brief GPIO digital port */
    volatile PORT_TYPE *port_register;
    
    /** @brief Mask 
     * 
     * Doesn't need to be volatile - it's read only once the struct 
     * has been initialized.
     */
    PINMASK_TYPE pin_mask;
};

static inline bool isValidIoPort(const ioPort &port) {
    return port.port_register!=nullptr;
}

/** @brief Get a null (empty) ioPort object.
 * 
 * @note Cannot be constexpr as the struct contains volatile members
 */
static inline ioPort nullIoPort(void) {
    return { nullptr, 0 };
}

/** Create & register a direct manipulation port */
ioPort pinToOutputPort(uint8_t pin);

/** Create & register a direct manipulation port */
ioPort pinToInputPort(uint8_t pin, uint8_t inputMode=INPUT /* INPUT or INPUT_PULLUP*/);

/** @brief Set an output pin low */
static CRITICAL_INLINE void setPin_Low(ioPort &port) {
    if (isValidIoPort(port)) {
        *port.port_register &= ~port.pin_mask;
    }
}

/** @brief Set an output pin low */
static CRITICAL_INLINE void setPin_Low(uint8_t pin) {
    if (isValidPin(pin)) {
        digitalWrite(pin, LOW);
    }
}

/** @brief Set an output pin high */
static CRITICAL_INLINE void setPin_High(ioPort &port) {
    if (isValidIoPort(port)) {
        *port.port_register |= port.pin_mask;
    }
}

/** @brief Set an output pin high */
static CRITICAL_INLINE void setPin_High(uint8_t pin) {
    if (isValidPin(pin)) {
        digitalWrite(pin, HIGH);
    }
}


/** @brief Toggle an output pin (low to high, high to low) */
static inline void togglePin(ioPort &port) {
    if (isValidIoPort(port)) {
        *port.port_register ^=  port.pin_mask;
    }
}

/** @brief Toggle an output pin (low to high, high to low) */
static inline void togglePin(uint8_t pin) {
    if (isValidPin(pin)) {
        digitalWrite(pin, !digitalRead(pin));
    }
}

/** @brief Set the pin high or low - but check it's a valid pin first */
static CRITICAL_INLINE void setPinState(uint8_t pinNumber, uint8_t state) {
    if (isValidPin(pinNumber)) {
        digitalWrite(pinNumber, state);
    }
}

/** @brief Set the pin high or low - but check it's a valid pin first */
static CRITICAL_INLINE void setPinState(ioPort &port, uint8_t state) {
    if (state==HIGH) {
        setPin_High(port);
    } else {
        setPin_Low(port);
    }
}

static inline uint8_t readPin(ioPort &port) {
    if (isValidIoPort(port)) {
        return (*port.port_register & port.pin_mask) ? HIGH : LOW;
    }
    return LOW;
}

static inline uint8_t readPin(uint8_t pin) {
    if (isValidPin(pin)) {
        return digitalRead(pin);
    }
    return LOW;
}

/** @brief Translate between the pin list that appears in TS and the actual pin numbers.
 * 
 * For the **digital IO**, this will simply return the same number as the rawPin value as those are mapped directly.
 * For **analog pins**, it will translate them into the correct internal pin number.
 * @param rawPin - High level pin number
 * @return Translated / usable pin number or PIN_INVALID
 */
uint8_t pinTranslate(uint8_t rawPin);

/** @brief Translate a pin number (0 - 22) to the relevant Ax (analog) pin reference.
 * 
 * This is required as some ARM chips do not have all analog pins in order (EG pin A15 != A14 + 1).
 */
uint8_t pinTranslateAnalog(uint8_t rawPin);