
/** \file
 * @brief A saturation cast
 * 
 * Loosely based on 
 * https://github.com/p-groarke/clamp_cast/tree/master/include/clamp_cast
 * https://github.com/chromium/chromium/blob/main/base/numerics/safe_conversions.h
 */

#pragma once

#include <stdint.h>
#include <limits>
#include <type_traits>
#if defined(ARDUINO_ARCH_AVR)
#include <bits/no_min_max.h>
#endif

/** @cond */
namespace saturated_cast_detail {

enum sign_conversion { Signed_Unsigned, Unsigned_Signed, NoSignConversion };
template <class In, class Out> struct direction {
    static constexpr sign_conversion value = std::is_signed<In>::value == std::is_signed<Out>::value ?
        sign_conversion::NoSignConversion : std::is_signed<In>::value ? Signed_Unsigned : Unsigned_Signed;
};

enum width_change { Narrowing, Widening, None };
template <class In, class Out> struct width_direction {
    static constexpr width_change value = sizeof(In)==sizeof(Out) ? width_change::None :
                                            sizeof(In)>sizeof(Out) ? width_change::Narrowing : 
                                                width_change::Widening;
};

template <class In, class Out, sign_conversion direction, width_change narrowing>
struct clamp_inner;

template <class In, class Out>
struct clamp_inner<In, Out, Unsigned_Signed, width_change::Narrowing> {
	static constexpr In hi = static_cast<In>(std::numeric_limits<Out>::max());

    static inline constexpr Out clamp(In in) {
        return static_cast<Out>(in>hi ? hi : in);
    }
};
template <class In, class Out>
struct clamp_inner<In, Out, Unsigned_Signed, width_change::Widening> {
    static inline constexpr Out clamp(In in) {
        return static_cast<Out>(in);
    }
};
template <class In, class Out>
struct clamp_inner<In, Out, Unsigned_Signed, width_change::None> {
	static constexpr In hi = static_cast<In>(std::numeric_limits<Out>::max());

    static inline constexpr Out clamp(In in) {
        return  static_cast<Out>(in>hi ? hi :in);
    }
};

template <class In, class Out>
struct clamp_inner<In, Out, Signed_Unsigned, width_change::Narrowing> {
	static constexpr In lo = static_cast<In>(0);
	static constexpr In hi = static_cast<In>(std::numeric_limits<Out>::max());

    static inline constexpr Out clamp(In in) {
        return static_cast<Out>(in<lo ? lo : in>hi ? hi : in);
    }
};
template <class In, class Out>
struct clamp_inner<In, Out, Signed_Unsigned, width_change::Widening> {
	static constexpr In lo = static_cast<In>(0);

    static inline constexpr Out clamp(In in) {
        return static_cast<Out>(in<lo ? lo : in);
    }
};
template <class In, class Out>
struct clamp_inner<In, Out, Signed_Unsigned, width_change::None> {
	static constexpr In lo = static_cast<In>(0);

    static inline constexpr Out clamp(In in) {
        return static_cast<Out>(in<lo ? lo : in);
    }
};

template <class In, class Out>
struct clamp_inner<In, Out, NoSignConversion, width_change::Narrowing> {
	static constexpr In lo = static_cast<In>(std::numeric_limits<Out>::min());
	static constexpr In hi = static_cast<In>(std::numeric_limits<Out>::max());

    static inline constexpr Out clamp(In in) {
        return static_cast<Out>(in<lo ? lo : in>hi ? hi : in);
    }
};
template <class In, class Out>
struct clamp_inner<In, Out, NoSignConversion, width_change::Widening> {
    static inline constexpr Out clamp(In in) {
        return static_cast<Out>(in);
    }
};
template <class In, class Out>
struct clamp_inner<In, Out, NoSignConversion, width_change::None> {
    static inline constexpr Out clamp(In in) {
        return static_cast<Out>(in);
    }
};
} // namespace detail
/** @endcond */

/** @brief A saturation cast
 * 
 * @details
 * saturated_cast<> is analogous to static_cast<> for numeric types, except
 * that the specified numeric conversion will saturate by default rather than
 * overflow or underflow.
 * 
 * See https://en.wikipedia.org/wiki/Saturation_arithmetic
 * 
 * Examples:
 * @code uint8_t u8 = saturated_cast<uint8_t>(-1); // u8 = 0 @endcode
 * @code uint8_t u8 = saturated_cast<uint8_t>(267); // u8 = 255 @endcode
 */
template <class Out, class In>
static inline constexpr Out saturated_cast(In in) {
	static_assert(std::is_arithmetic<In>::value,
			"saturate_cast : input type must be arithmetic");
	static_assert(std::is_arithmetic<Out>::value,
			"saturate_cast : output type must be arithmetic");

    return saturated_cast_detail::clamp_inner<
                In, Out, 
                saturated_cast_detail::direction<In, Out>::value, 
                saturated_cast_detail::width_direction<In, Out>::value>::clamp(in);
}

#if defined(ARDUINO_ARCH_AVR)
POP_MINMAX()
#endif