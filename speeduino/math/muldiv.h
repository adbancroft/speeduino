#pragma once

#include <stdint.h>
#include <type_traits>
#include "division.h"

/** \file
 * @brief Optimised calculation of (a*b)/c
 */
 
/**
 * @defgroup muldiv muldiv
 * @brief Optimised calculation of (a*b)/c
 *
 * This family of functions will calculate (a*b)/c using the narrowest type possible,
 * which should be the fastest method on AVR. Division is implemented in
 * software so narrower types == faster division.
 * 
 * (a*b)/c is often used to scale a value from one range to another.
 * 
 * Note: The return type is the same as the inputs, so <b>THESE SHOULD NOT BE CALLED IF THE RESULT WOULD OVERFLOW THE RETURN TYPE</b>
 * 
 * @param a Multiplier 1    
 * @param b Multiplier 2
 * @param c The divisor
 * @return The result of (a*b)/c
 * @{
 * */
static inline uint8_t muldiv(uint8_t a, uint8_t b, uint8_t div)
{
    return udiv_16_8((uint16_t)a * (uint16_t)b, div);
}

static inline uint16_t muldiv(uint16_t a, uint16_t b, uint16_t div)
{
    return udiv_32_16((uint32_t)a * (uint32_t)b, div);
}

/** @} */