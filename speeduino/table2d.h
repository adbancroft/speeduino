#pragma once

#include <stdint.h>
#include <type_traits>
#include "currentstatus.h"
#include "maths.h"
#include "find_bin.h"

template <typename axis_t, typename value_t>
struct table2d_lookup_cache 
{
  static_assert(std::is_integral<axis_t>::value, "T must be an integral type");
  static_assert(std::is_integral<value_t>::value, "T must be an integral type");

  uint8_t lastXMax = 1U; // The axis bin search algo relies on this being 1 initially

  //Store the last input and output for caching
  axis_t lastInput;
  value_t lastOutput;
  uint8_t cacheTime = UINT8_MAX; //Tracks when the last cache value was set so it can expire after x seconds. A timeout is required to pickup when a tuning value is changed, otherwise the old cached value will continue to be returned as the X value isn't changing. 
};

template <typename axis_t, typename value_t, uint8_t sizeT>
struct table2d
{
  static_assert(std::is_integral<axis_t>::value, "T must be an integral type");
  static_assert(std::is_integral<value_t>::value, "T must be an integral type");

  static constexpr uint8_t size = sizeT;
  typedef axis_t axis_type;
  typedef value_t value_type;

  value_t *values;
  axis_t *axisX;
  table2d_lookup_cache<axis_t, value_t> cache;

  table2d(axis_t (&axisBin)[sizeT], value_t (&curve)[sizeT])
    : values(curve), axisX(axisBin)
  {
  }
};

typedef table2d<uint8_t, uint8_t, 4> table2du8u8_4;
typedef table2d<int8_t, uint8_t, 4> table2di8u8_4;
typedef table2d<uint8_t, uint8_t, 10> table2du8u8_10;
typedef table2d<uint8_t, uint8_t, 6> table2du8u8_6;
typedef table2d<uint8_t, uint8_t, 9> table2du8u8_9;
typedef table2d<uint8_t, uint8_t, 8> table2du8u8_8;
typedef table2d<uint8_t, uint16_t, 4> table2du8u16_4;
typedef table2d<uint8_t, int16_t, 6> table2du8s16_6;
typedef table2d<uint16_t, uint16_t, 32> table2du16u16_32;
typedef table2d<uint16_t, uint8_t, 32> table2du16u8_32;

static inline uint8_t getCacheTime() {
  return currentStatus.secl;
}

/*
 * @brief Lookup a value in a curve, interpolating if necessary
 */
template <typename axis_t, typename value_t, uint8_t sizeT>
value_t table2D_getValue(table2d<axis_t, value_t, sizeT> *fromTable, axis_t X_in)
{
  static_assert(std::is_integral<axis_t>::value, "T must be an integral type");
  static_assert(std::is_integral<value_t>::value, "T must be an integral type");

  //Check whether the X input is the same as last time this ran
  if( (X_in != fromTable->cache.lastInput) || (fromTable->cache.cacheTime != getCacheTime()) )
  {
    fromTable->cache.lastXMax = find_bin(X_in, fromTable->axisX, 0, sizeT-1, fromTable->cache.lastXMax);
    fromTable->cache.lastOutput = rescale(X_in, 
                                          fromTable->axisX[fromTable->cache.lastXMax-1], fromTable->axisX[fromTable->cache.lastXMax], 
                                          fromTable->values[fromTable->cache.lastXMax - 1], fromTable->values[fromTable->cache.lastXMax]);
    fromTable->cache.lastInput = X_in;
    fromTable->cache.cacheTime = getCacheTime(); //As we're not using the cache value, set the current secl value to track when this new value was calc'd
  }

  return fromTable->cache.lastOutput;
}

#include "saturated_cast.hpp"

/*
 * @brief Lookup a value in a curve, interpolating if necessary
 */
template <typename axis_t, typename value_t, uint8_t sizeT, typename axis_query_t>
inline value_t table2D_getValue(table2d<axis_t, value_t, sizeT> *fromTable, axis_query_t X_in)
{
  static_assert(std::is_integral<axis_t>::value, "T must be an integral type");
  static_assert(std::is_integral<value_t>::value, "T must be an integral type");
  static_assert(std::is_integral<axis_query_t>::value, "T must be an integral type");

  // This function is only here to skip casting in callers if necessary
  
  // Clamp the input to the range of the axis type. Using a cast would reinterpret the bit pattern
  // which isn't what we want.
  axis_t X = saturated_cast<axis_t>(X_in);
  return table2D_getValue<axis_t, value_t, sizeT>(fromTable, X);
}
