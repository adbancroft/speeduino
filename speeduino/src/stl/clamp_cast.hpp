/**
 * BSD 3-Clause License
 *
 * Copyright (c) 2019, Philippe Groarke
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 **/

#pragma once
#include <stdint.h>
#include <limits>
#include <type_traits>
#include <bits/no_min_max.h>

namespace detail {

template <typename T, bool, bool> struct big_max_helper;
template <typename T> struct big_max_helper<T, false, false> {
    typedef uintmax_t value_type;
    static constexpr value_type value = static_cast<value_type>(std::numeric_limits<T>::max());
};
template <typename T> struct big_max_helper<T, true, false> {
    typedef long double value_type;
    static constexpr value_type value = static_cast<value_type>(std::numeric_limits<T>::max());
};
template <typename T> struct big_max_helper<T, false, true> {
    typedef intmax_t value_type;
    static constexpr value_type value = static_cast<value_type>(std::numeric_limits<T>::max());
};

template <class In, class Out>
struct clamp_hi_in {
	static constexpr In low = static_cast<In>(std::numeric_limits<Out>::lowest());
	static constexpr In hi = static_cast<In>(std::numeric_limits<Out>::max());

	static Out clamp(In in) {
		return in > hi ? std::numeric_limits<Out>::max() : 
			in < low ? std::numeric_limits<Out>::lowest() : static_cast<Out>(in);
	}
};

template <class InUnsigned, class Out>
struct clamp_hi_in_unsigned {
	static constexpr InUnsigned low = 0;
	static constexpr InUnsigned hi = static_cast<InUnsigned>(std::numeric_limits<Out>::max());

	static Out clamp(InUnsigned in) {
		return in > hi ? std::numeric_limits<Out>::max() : in < low ? static_cast<Out>(low) : static_cast<Out>(in);
	}
};
} // namespace detail

template <class Out, class In>
[[nodiscard]] Out clamp_cast(In in) {
	static_assert(std::is_arithmetic<In>::value,
			"saturate_cast : input type must be arithmetic");
	static_assert(std::is_arithmetic<Out>::value,
			"saturate_cast : output type must be arithmetic");

	constexpr auto in_max = detail::big_max_helper<In, std::is_floating_point<In>::value, std::is_signed<In>::value>::value;
	constexpr auto out_max = detail::big_max_helper<Out, std::is_floating_point<Out>::value, std::is_signed<Out>::value>::value;

	if (std::is_same<In, Out>::value) {
		return in;
	} else if (std::is_unsigned<In>::value && std::is_signed<Out>::value) {
		if (in_max > out_max) {
			return detail::clamp_hi_in<In, Out>::clamp(in);
		} else {
			return static_cast<Out>(in);
		}
	} else if (std::is_signed<In>::value && std::is_unsigned<Out>::value) {
		if (in_max > out_max) {
			return detail::clamp_hi_in_unsigned<In, Out>::clamp(in);
		} else {
			if (in < static_cast<In>(0)) {
				return Out{ 0 };
			}
			return static_cast<Out>(in);
		}
	} else {
		if (in_max > out_max) {
			return detail::clamp_hi_in<In, Out>::clamp(in);
		} else {
			return static_cast<Out>(in);
		}
	}
}

POP_MINMAX()