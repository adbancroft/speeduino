#pragma once

/** @file
 * @brief Board related definitions and inclusion of board specific header files.
 * 
 * @note This file should be named "board.h", but one of the STM32 Arduino implementations
 * has a <board.h> include. Which picks up *this file* instead of the intended file :-( 
*/

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
  #define BOARD_MAX_DIGITAL_PINS 54U //digital pins +1
  #define BOARD_MAX_IO_PINS 70U //digital pins + analog channels + 1
  #define BOARD_MAX_ADC_PINS  15U //Number of analog pins
  #ifndef LED_BUILTIN
    #define LED_BUILTIN 13
  #endif
  #define CORE_AVR
  #define BOARD_H "board_avr2560.h"
  #ifndef INJ_CHANNELS
    #define INJ_CHANNELS 4
  #endif
  #ifndef IGN_CHANNELS
    #define IGN_CHANNELS 5
  #endif

  #if defined(__AVR_ATmega2561__)
    //This is a workaround to avoid having to change all the references to higher ADC channels. We simply define the channels (Which don't exist on the 2561) as being the same as A0-A7
    //These Analog inputs should never be used on any 2561 board definition (Because they don't exist on the MCU), so it will not cause any issues
    #define A8  A0
    #define A9  A1
    #define A10  A2
    #define A11  A3
    #define A12  A4
    #define A13  A5
    #define A14  A6
    #define A15  A7
  #endif

  //#define TIMER5_MICROS

#elif defined(CORE_TEENSY)
  #if defined(__MK64FX512__) || defined(__MK66FX1M0__)
    #define CORE_TEENSY35
    #define BOARD_H "board_teensy35.h"
    #define BOARD_MAX_ADC_PINS  22 //Number of analog pins
  #elif defined(__IMXRT1062__)
    #define CORE_TEENSY41
    #define BOARD_H "board_teensy41.h"
    #define BOARD_MAX_ADC_PINS  17 //Number of analog pins
  #endif
  #define INJ_CHANNELS 8
  #define IGN_CHANNELS 8

#elif defined(STM32_MCU_SERIES) || defined(ARDUINO_ARCH_STM32) || defined(STM32)
  #define BOARD_H "board_stm32_official.h"
  #define CORE_STM32

  #define BOARD_MAX_ADC_PINS  NUM_ANALOG_INPUTS-1 //Number of analog pins from core.
  #if defined(STM32F407xx) //F407 can do 8x8 STM32F401/STM32F411 don't
   #define INJ_CHANNELS 8
   #define IGN_CHANNELS 8
  #else
   #define INJ_CHANNELS 4
   #define IGN_CHANNELS 5
  #endif
#elif defined(__SAMD21G18A__)
  #define BOARD_H "board_samd21.h"
  #define CORE_SAMD21
  #define CORE_SAM
  #define INJ_CHANNELS 4
  #define IGN_CHANNELS 4
#elif defined(__SAMC21J18A__)
  #define BOARD_H "board_samc21.h"
  #define CORE_SAMC21
  #define CORE_SAM
#elif defined(__SAME51J19A__)
  #define BOARD_H "board_same51.h"
  #define CORE_SAME51
  #define CORE_SAM
  #define INJ_CHANNELS 8
  #define IGN_CHANNELS 8
#else
  #error Incorrect board selected. Please select the correct board (Usually Mega 2560) and upload again
#endif

#include <Arduino.h>
//This can only be included after the above section
#include BOARD_H //Note that this is not a real file, it is defined above

#if defined(CORE_TEENSY)
#define OUTPUT_CONTROL_SUPPORTED
#endif

/**
 * @brief Get the timer tick interval for the target CPU
 * 
 * @return uint8_t Timer interval in microseconds
 */
static inline uint8_t getTimerInterval(void) {
#if defined(TIMER_RESOLUTION)
  return TIMER_RESOLUTION;
#elif defined(CORE_AVR)
  return 16U;
#elif defined(CORE_TEENSY35)
  return 32U;
#elif defined(CORE_TEENSY41)
  return 2U;
#else
  static_assert(false, "Unknown timer tick interval");
#endif
}

