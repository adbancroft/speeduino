#pragma once

#include "board_definition.h"

// AVR & Teensy both support the AVR atomic macros
#if defined(CORE_AVR) || defined(CORE_TEENSY)
#include <util/atomic.h>
#else
// For other architectures, our fallback is to turn interrupts off & on via the Arduino API
// I.e. ATOMIC_BLOCK(ATOMIC_RESTORESTATE) is equivalent to ATOMIC_BLOCK(ATOMIC_FORCEON)
// As of writing this code, the Arduino API doesn't have a function to test if interrupts are 
// on or off :-(

#include <stdint.h>
#include <Arduino.h>

/* Internal helper functions. */
static __inline__ uint8_t __iCliRetVal(void)
{
    noInterrupts();
    return 1;
}
static __inline__ void __iSeiParam(const uint8_t *__s)
{
    interrupts();
    __asm__ volatile ("" ::: "memory");
    (void)__s;
}

#define ATOMIC_BLOCK(type) \
  for ( type, __ToDo = __iCliRetVal(); __ToDo ; __ToDo = 0 )
#define ATOMIC_FORCEON \
  uint8_t sreg_save __attribute__((__cleanup__(__iSeiParam))) = 0
#define	ATOMIC_RESTORESTATE ATOMIC_FORCEON

#endif