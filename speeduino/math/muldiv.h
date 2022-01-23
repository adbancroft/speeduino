#pragma once

#include <stdint.h>
#include <type_traits>
#include "division.h"

/** \file
 * @brief Optimised calculation of (a*b)/c
 */
 
/** @cond */
namespace muldiv_detail {
template <typename T, typename I>
inline T muldiv_simple(T a, T b, T div)
{
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    return ((I)a * b) / div;

    // This version is applies the correct rounding (+- c/2), but slower & uses more memory
    // I numerator = ((I)a * b);
    // T offset = numerator<0 ? -c/2 : c/2;
    // return (numerator + offset) / c;    
}
} // namespace detail
/** @endcond */

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
 * Notes:
 * 1. The return type is the same as the inputs, so <b>THESE SHOULD NOT BE CALLED IF THE RESULT WOULD OVERFLOW THE RETURN TYPE</b>
 * 2. The optimization only works on 16-bit numbers and only for small values of a and b (less than sqrt([u]int16_t])).
 *    Additional overloads for other types are provided for uniformity.
 * 
 * @param a Multiplier 1    
 * @param b Multiplier 2
 * @param c The divisor
 * @return The result of (a*b)/c
 * @{
 * */
inline int8_t muldiv(int8_t a, int8_t b, int8_t div)
{
    // By the time we condtionally check for potential overflow, including
    // accounting for negative operands, we are just as quick promoting
    // to int16_t & applying the multiply + divide.
    return muldiv_detail::muldiv_simple<int8_t, int16_t>(a, b, div);
}

inline uint8_t muldiv(uint8_t a, uint8_t b, uint8_t div)
{
    // By the time we condtionally check for potential overflow,
    // we are just as quick promoting to int16_t & applying the 
    // multiply + divide.
    return muldiv_detail::muldiv_simple<uint8_t, uint16_t>(a, b, div);
}

inline int16_t muldiv(const int16_t a, const int16_t b, const int16_t div)
{
    constexpr int16_t overflow_threshold = 182; // sqrt(INT16_MAX) + 1
    if (__builtin_abs(a)<overflow_threshold && __builtin_abs(b)<overflow_threshold)
    {
        // Fast path - avoid int32_t promotion
        return muldiv_detail::muldiv_simple<int16_t, int16_t>(a, b, div);
    }
    return muldiv_detail::muldiv_simple<int16_t, int32_t>(a, b, div);
}

inline uint16_t muldiv(const uint16_t a, const uint16_t b, const uint16_t div)
{
    return udiv_32_16((uint32_t)a * (uint32_t)b, div);
}

inline int32_t muldiv(const int32_t a, const int32_t b, const int32_t div)
{
    return muldiv_detail::muldiv_simple<int32_t, int32_t>(a, b, div);
}

inline uint32_t muldiv(const uint32_t a, const uint32_t b, const uint32_t div)
{
    uint32_t dividend = a * b;
#ifdef USE_LIBDIVIDE    
    if (div<=(uint32_t)UINT16_MAX && udiv_is16bit_result(dividend, div)) {
        return udiv_32_16(dividend, (uint16_t)div);
    }
#endif
    return dividend / div;
}

/** @} */