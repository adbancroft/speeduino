
#pragma once

#include <stdint.h>
#include "maths.h"

template <typename T>
T constexpr intermediate(T const& min, T const& max, uint8_t const& frac)
{
  return min + muldiv((T)(max - min), (T)frac, (T)100U);
}