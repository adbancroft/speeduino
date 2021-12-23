#pragma once

#include <stdint.h>
#if !defined(UNIT_TEST)
#include "currentstatus.h"
#endif
#include "src/stl/type_traits"
#include "maths.h"
#include "array_utils.h"

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

template <typename axis_t, typename value_t, uint8_t sizeT>
static inline value_t interpolate(const table2D<axis_t, value_t, sizeT> *fromTable, const axis_t X, const axis_t xMaxValue, const axis_t xMinValue)
{
  /* Float version (if m, yMax, yMin and n were float's)
      int yVal = (m * (yMax - yMin)) / n;
  */

  typedef typename std::make_unsigned<axis_t>::type unsigned_axis_t;
  typedef typename std::make_unsigned<value_t>::type unsigned_value_t;
  // Pick the wider of the two types to use for the calculation
  // Note that std::common_type will not do what we want here, as it will use integer
  // calculation promotion rules. So everything will be "int" at a minimum.
  typedef typename std::conditional<(sizeof(unsigned_axis_t) >= sizeof(unsigned_value_t)), unsigned_axis_t, unsigned_value_t>::type u_common_t;

  value_t yMax = fromTable->values[fromTable->cache.lastXMax];
  value_t yMin = fromTable->values[fromTable->cache.lastXMax - 1];
  // We convert everything to unsigned for performance reasons
  // and convert back to signed at the end.
  int8_t multiplier = yMax<yMin ? -1 : 1;

  u_common_t binDistance = X - xMinValue;
  u_common_t binWidth = xMaxValue - xMinValue;

  u_common_t valueWidth = multiplier * (yMax - yMin);
  u_common_t scaled = muldiv(valueWidth, binDistance, binWidth);
  return yMin + (multiplier * scaled);
}

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
      fromTable->cache.lastOutput = fromTable->values[fromTable->cache.lastXMax-1];
    }
    else
    {
      fromTable->cache.lastOutput = interpolate(fromTable, X, xMaxValue, xMinValue);
    }
  }

  fromTable->cache.cacheTime = getCacheTime(); //As we're not using the cache value, set the current secl value to track when this new value was calc'd
  fromTable->cache.lastInput = X_in;
  return fromTable->cache.lastOutput;
}

#include <clamp_cast.hpp>
#include <bits/no_min_max.h>

template <typename axis_t, typename value_t, uint8_t sizeT, typename axis_query_t>
inline value_t table2D_getValue(table2D<axis_t, value_t, sizeT> *fromTable, axis_query_t X_in)
{
  axis_t X = clamp_cast<axis_t>(X_in);
  return table2D_getValue<axis_t, value_t, sizeT>(fromTable, X);
}

POP_MINMAX()