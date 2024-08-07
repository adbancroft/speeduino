#pragma once
#include "maths.h"

/**
 * @file
 * @brief A faster implementation of the Arduino map() function.
 * 
 * Takes advantage of our optimized unsigned division functions.
 *
 */

/// @cond

// Below namespaces are private details of the fast map implementation 

// A poor, but adequate, replacement for type traits templates in the
// C++ standard library.
//
// Note that these are all compile time operations.
namespace type_traits {
    // Limited replacement for std::make_unsigned 
    template< typename T > struct make_unsigned;
    template< > struct make_unsigned<int8_t> {
        typedef uint8_t type;
    };
    template< > struct make_unsigned<uint8_t> {
        typedef uint8_t type;
    };
    template< > struct make_unsigned<int16_t> {
        typedef uint16_t type;
    };
    template< > struct make_unsigned<uint16_t> {
        typedef uint16_t type;
    };

    template< typename T >
    using make_unsigned_t = typename make_unsigned<T>::type;

    // Limited replacement for std::conditional
    // Primary template for true
    template<bool _Cond, typename _Iftrue, typename _Iffalse>
        struct conditional
        { typedef _Iftrue type; };

    // Partial specialization for false.
    template<typename _Iftrue, typename _Iffalse>
        struct conditional<false, _Iftrue, _Iffalse>
        { typedef _Iffalse type; };    

}

// 
namespace fast_map_impl {

    // Widen an integral type to the next larger type.
    // This is used to avoid overflow during the calculation.
    template < typename T > struct widen_integral { } ;
    template< > struct widen_integral<uint8_t> {
        typedef uint16_t type;
    };
    template< > struct widen_integral<uint16_t> {
        typedef uint32_t type;
    };
    template< > struct widen_integral<uint32_t> {
        typedef uint64_t type;
    };
    template< typename T >
    using widen_integral_t = typename widen_integral<T>::type;
 
    // Get the absolute difference between two values.
    // This is used to handle negative ranges.
    // Equivalent of abs(min-max)
    template <typename T>
    static inline type_traits::make_unsigned_t<T> absDelta(const T &min, const T &max) {
        if (max<min) {
            return min - max;
        }
        return max - min;
    }

}

/// @endcond

/**
 * @brief Optimized version of Arduino's map() function.
 * 
 * * For 8-bit values (used in many 2d tables), it's 6x faster
 * * For 16-bit values, it's 3x faster
 * * For 8-bit to 16-bit values, it's 2.5x faster
 * 
 * (see unit tests)
 * 
 * The only difference from map() is that it will clip to the range min/max.
 * 
 * @tparam TIn Input range type
 * @tparam TOut Output range type
 * @param in Input value
 * @param inMin Input range minimum
 * @param inMax Input range maximum
 * @param outMin Output range minimum
 * @param outMax Output range maximum
 * @return TOut
 */
template <typename TIn, typename TOut>
static inline TOut fastMap(TIn in, TIn inMin, TIn inMax, TOut outMin, TOut outMax) {
    /* Float version (if m, yMax, yMin and n were float's)
        int yVal = (m * (yMax - yMin)) / n;
    */

    // We use unsigned types for performance and code simplicity.
    typedef typename type_traits::make_unsigned_t<TIn> in_unsigned_t;
    typedef typename type_traits::make_unsigned_t<TOut> out_unsigned_t;

    // Pick the wider of the two types to use for the calculation
    // Note that std::common_type will not do what we want here, as it will use integer
    // calculation promotion rules. So everything will be "int" at a minimum - our
    // goal is to use the narrowest type possible for performance reasons.
    typedef typename type_traits::conditional<(sizeof(in_unsigned_t) >= sizeof(out_unsigned_t)), in_unsigned_t, out_unsigned_t>::type u_common_t;

    // Finally widen the unsigned common type - this is our intermediate calculation type
    // to avoid overflow.
    typedef typename fast_map_impl::widen_integral_t<u_common_t> u_intermediate_t;

    // Clip to range if necessary
    if (((inMin<inMax) && (in<inMin)) || 
        // In case of an inverted range
        ((inMax<inMin) && (in>inMin))) {
            return outMin;
    }
    if (((inMin<inMax) && (in>inMax)) || 
        // In case of an inverted range
        ((inMax<inMin) && (in<inMax))) {
            return outMax;
    }

    // Note that this is all unsigned math.
    if (((inMin<inMax) && (in>=inMin && in<=inMax)) || 
        // In case of an inverted range
        ((inMax<inMin) && (in>=inMax && in<=inMin))) {

        in_unsigned_t m = fast_map_impl::absDelta(inMin, in);
        in_unsigned_t inRange = fast_map_impl::absDelta(inMin, inMax);
        out_unsigned_t outRange = fast_map_impl::absDelta(outMin, outMax);
        out_unsigned_t scaled = uFastDiv((u_intermediate_t)m * (u_intermediate_t)outRange, inRange);
        // If the out range is descending we need to invert the offset.  
        if (outMax<outMin) {
            return outMin - scaled;
        }
        return outMin + scaled;     
    }
    return outMin;
}
