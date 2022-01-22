#ifndef MATH_H
#define MATH_H

#include <stdint.h>
#include <type_traits>
#include "math/division.h"
#include "math/FixedPoint.h"
#include "math/muldiv.h"
#include "math/rescale.h"
#include "math/saturated_cast.hpp"

#ifdef USE_LIBDIVIDE
// We use pre-computed constant parameters with libdivide where possible. 
// Using predefined constants saves flash and RAM (.bss) versus calling the 
// libdivide generator functions (E.g. libdivide_s32_gen)
// 32-bit constants generated here: https://godbolt.org/z/vP8Kfejo9
#include "src/libdivide/libdivide.h"
#endif

extern uint8_t random1to100(void);


/**
 * @brief Integer based percentage calculation.
 * 
 * @param percent The percent to calculate ([0, 100])
 * @param value The value to operate on
 * @return uint32_t 
 */
static inline uint32_t percentage(uint8_t percent, uint32_t value) 
{
    return (uint32_t)div100((uint32_t)value * (uint32_t)percent);
}


/**
 * @brief Integer based half-percentage calculation.
 * 
 * @param percent The percent to calculate ([0, 100])
 * @param value The value to operate on
 * @return uint16_t 
 */
static inline uint16_t halfPercentage(uint8_t percent, uint16_t value) {
    uint32_t x200 = (uint32_t)percent * (uint32_t)value;
#ifdef USE_LIBDIVIDE    
    return (uint16_t)libdivide::libdivide_u32_do_raw(x200 + DIV_ROUND_CORRECT(UINT32_C(200), uint32_t), 2748779070L, 7);
#else
    return (uint16_t)UDIV_ROUND_CLOSEST(x200, UINT16_C(200), uint32_t);
#endif
}

/**
 * @brief Make one pass at correcting the value into the range [min, max)
 * 
 * @param min Minimum value (inclusive)
 * @param max Maximum value (exclusive)
 * @param value Value to nudge
 * @param nudgeAmount Amount to change value by 
 * @return int16_t 
 */
static inline int16_t nudge(int16_t min, int16_t max, int16_t value, int16_t nudgeAmount)
{
    if (value<min) { return value + nudgeAmount; }
    if (value>max) { return value - nudgeAmount; }
    return value;
}

/**
 * @brief clamps a given value between the minimum and maximum thresholds.
 * 
 * Uses operator< to compare the values.
 * 
 * @tparam T Any type that supports operator<
 * @param v The value to clamp 
 * @param lo The minimum threshold
 * @param hi The maximum threshold
 * @return if v compares less than lo, returns lo; otherwise if hi compares less than v, returns hi; otherwise returns v.
 */
template<class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi){
    return v<lo ? lo : hi<v ? hi : v;
}

/**
 * @brief Simple low pass IIR filter for U16 values
 * 
 * This is effectively implementing the smooth filter from playground.arduino.cc/Main/Smooth
 * But removes the use of floats and uses 8 bits of fixed precision.
 */
static inline uint16_t LOW_PASS_FILTER(uint16_t input, uint8_t alpha, uint16_t prior) {
  // Intermediate steps are for MISRA compliance
  // Equivalent to: (input * (256 - alpha) + (prior * alpha)) >> 8
  uint16_t inv_alpha = 256U - (uint16_t)alpha;
  uint32_t prior_alpha = (prior * (uint32_t)alpha);
  uint32_t preshift = (input * (uint32_t)inv_alpha) + prior_alpha;
  return preshift >> 8U;
}

/** @brief Simple low pass IIR filter for S16 values */
static inline int16_t LOW_PASS_FILTER(int16_t input, uint8_t alpha, int16_t prior) {
  // Intermediate steps are for MISRA compliance
  // Equivalent to: (input * (256 - alpha) + (prior * alpha)) >> 8
  int16_t inv_alpha = 256 - (int16_t)alpha;
  int32_t prior_alpha = (prior * (int32_t)alpha);
  int32_t preshift = ((input * (int32_t)inv_alpha) + prior_alpha);
  return (uint32_t)preshift >> 8U;
}

#endif