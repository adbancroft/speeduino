#pragma once

#include <type_traits>
#include "muldiv.h"
#include "bit_shifts.h"

/** \file
 * @brief Optimised integer rescaling
 */


//This is a dedicated function that specifically handles the case of mapping 0-1023 values into a 0 to X range
//This is a common case because it means converting from a standard 10-bit analog input to a byte or 10-bit analog into 0-511 (Eg the temperature readings)
#define fastMap1023toX(x, out_max) ( rshift<10>((uint32_t)(x) * (out_max)) )
//This is a new version that allows for out_min
#define fastMap10Bit(x, out_min, out_max) ( rshift<10>( (uint32_t)(x) * ((out_max)-(out_min)) ) + (out_min))

/* @brief Integer rescaling from one range to another
 *
 * Map a value from one range to another. The ranges can have different types.
 * Assumes the input range is ascending.
 * The output range can be either ascending or descending.
 * 
 * @param fromValue the value to rescale
 * @param fromMin the minimum value of the input range
 * @param fromMax the maximum value of the input range
 * @param toMin the minimum value of the output range
 * @param toMax the maximum value of the output range
 */
template <typename from_t, typename to_t>
static inline to_t rescale(const from_t fromValue, const from_t fromMin, const from_t fromMax, const to_t toMin, const to_t toMax) 
{
    static_assert(std::is_integral<from_t>::value, "from_t must be an integral type");
    static_assert(std::is_integral<to_t>::value, "to_t must be an integral type");

    /* Float version, if parameters were floats
        int result = (fromValue * (toMax - toMin)) / (fromMax-fromMin);
    */

    // We use unsigned types for performance and code simplicity.
    typedef typename std::make_unsigned<from_t>::type unsigned_axis_t;
    typedef typename std::make_unsigned<to_t>::type unsigned_value_t;
    
    // Pick the wider of the two types to use for the calculation
    // Note that std::common_type will not do what we want here, as it will use integer
    // calculation promotion rules. So everything will be "int" at a minimum - our
    // goal is to use the narrowest type possible for performance reasons.
    typedef typename std::conditional<(sizeof(unsigned_axis_t) >= sizeof(unsigned_value_t)), unsigned_axis_t, unsigned_value_t>::type u_common_t;

    // Clamp to output range
    if (fromValue<=fromMin) { return toMin; }
    if (fromValue>=fromMax) { return toMax; }

    u_common_t fromDistance = fromValue - fromMin;
    u_common_t fromWidth = fromMax - fromMin;

    // If the scale-to range is inverted we convert to unsigned and invert the result
    // This helps performance and simplifies the code.
    if (toMax<toMin)
    {
        u_common_t toWidth = toMin - toMax;
        u_common_t scaled = muldiv(toWidth, fromDistance, fromWidth);
        return toMin - scaled;
    }
    u_common_t toWidth = toMax - toMin;
    u_common_t scaled = muldiv(toWidth, fromDistance, fromWidth);
    return toMin + scaled;  
}
