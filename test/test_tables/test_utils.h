
#pragma once

#include <stdint.h>
#include "maths.h"

template <typename T>
T constexpr intermediate(T const& min, T const& max, uint8_t const& frac)
{
  return min + percentage(frac, (max - min));
}