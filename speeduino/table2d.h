#pragma once

#include <stdint.h>
using byte = uint8_t;
#if !defined(UNIT_TEST)
#include "currentstatus.h"
#endif
#include <type_traits>
#include "maths.h"
#include "find_bin.h"

template <typename axis_t, typename value_t>
struct table2D_lookup_cache 
{
  uint8_t lastXMax = 1U; // The axis bin search algo relies on this being 1 initially

  //Store the last input and output for caching
  axis_t lastInput;
  value_t lastOutput;
  uint8_t cacheTime = UINT8_MAX; //Tracks when the last cache value was set so it can expire after x seconds. A timeout is required to pickup when a tuning value is changed, otherwise the old cached value will continue to be returned as the X value isn't changing. 
};

template <typename axis_t, typename value_t, uint8_t sizeT>
struct table2D
{
  static constexpr uint8_t size = sizeT;
  typedef axis_t axis_type;
  typedef value_t value_type;

  value_t *values;
  axis_t *axisX;

  table2D_lookup_cache<axis_t, value_t> cache;
};

static inline uint8_t getCacheTime(void) {
#if !defined(UNIT_TEST)
  return currentStatus.secl;
#else
  return 0;
#endif
}

/*
 * @brief Lookup a value in a curve, interpolating if necessary
 */
template <typename axis_t, typename value_t, uint8_t sizeT>
value_t table2D_getValue(table2D<axis_t, value_t, sizeT> *fromTable, axis_t X_in)
{
  //Check whether the X input is the same as last time this ran
  if( (X_in == fromTable->cache.lastInput) && (fromTable->cache.cacheTime == getCacheTime()) )
  {
    // No-op
  }
  else
  {
    fromTable->cache.cacheTime = getCacheTime(); //As we're not using the cache value, set the current secl value to track when this new value was calc'd
  
    axis_t X = X_in;
    fromTable->cache.lastXMax = find_bin(X, fromTable->axisX, 0, sizeT-1, fromTable->cache.lastXMax);
    axis_t xMaxValue = fromTable->axisX[fromTable->cache.lastXMax];
    axis_t xMinValue = fromTable->axisX[fromTable->cache.lastXMax-1];

    if (X==xMaxValue)
    {
      fromTable->cache.lastOutput = fromTable->values[fromTable->cache.lastXMax];
    }
    else if (X==xMinValue)
    {
      fromTable->cache.lastOutput = fromTable->values[fromTable->cache.lastXMax - 1];
    }
    else
    {
      fromTable->cache.lastOutput = rescale(X, xMinValue, xMaxValue, fromTable->values[fromTable->cache.lastXMax - 1], fromTable->values[fromTable->cache.lastXMax]);
    }
  }

  fromTable->cache.lastInput = X_in;
  return fromTable->cache.lastOutput;
}

#include "saturated_cast.hpp"

/*
 * @brief Lookup a value in a curve, interpolating if necessary
 */
template <typename axis_t, typename value_t, uint8_t sizeT, typename axis_query_t>
inline value_t table2D_getValue(table2D<axis_t, value_t, sizeT> *fromTable, axis_query_t X_in)
{
  // This function is only here to skip casting in callers if necessary
  
  // Clamp the input to the range of the axis type. Using a cast would reinterpret the bit pattern
  // which isn't what we want.
  axis_t X = saturated_cast<axis_t>(X_in);
  return table2D_getValue<axis_t, value_t, sizeT>(fromTable, X);
}
