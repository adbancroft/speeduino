#pragma once

/** \file
 * @brief Fixed point math support 
 */

#include <stdint.h>

/** @brief An unsigned fixed point number type with 1 integer bit & 8 fractional bits.
 * See https://en.wikipedia.org/wiki/Q_(number_format).
 * This is specialized for the number range 0..1 - a generic fixed point
 * class would miss some important optimizations. Specifically, we can avoid
 * type promotion during multiplication.
 * */
using QU1X8_t = uint16_t;
static constexpr uint8_t QU1X8_INTEGER_SHIFT = 8u;
static constexpr QU1X8_t QU1X8_ONE = (uint16_t)1U << (uint16_t)QU1X8_INTEGER_SHIFT;
static constexpr QU1X8_t QU1X8_HALF = (uint16_t)1U <<((uint16_t)QU1X8_INTEGER_SHIFT-1U);

/** @brief Multiply 2 QU1X8_t fractions */
static inline QU1X8_t mulQU1X8(QU1X8_t a, QU1X8_t b)
{
    // 1x1 == 1....but the real reason for this is to avoid 16-bit multiplication overflow.
    //
    // We are using uint16_t as our underlying fixed point type. If we follow the regular
    // code path, we'd need to promote to uint32_t to avoid overflow.
    //
    // The overflow can only happen when *both* a & b are one
    //
    // This is a rare condition, so most of the time we can use 16-bit mutiplication and gain performance
    if (a==QU1X8_ONE && b==QU1X8_ONE)
    {
        return QU1X8_ONE;
    }
  // Add the equivalent of 0.5 to the final calculation pre-rounding.
  // This will have the effect of rounding to the nearest integer, rather
  // than always rounding down.
  return ((a * b) + QU1X8_HALF) >> QU1X8_INTEGER_SHIFT;
}
