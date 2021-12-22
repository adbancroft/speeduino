#pragma once

#include <stdint.h>
#if !defined(UNIT_TEST)
#include "currentstatus.h"
#endif
// #include "src/stl/type_traits.hpp"
// #include "src/stl/limits.hpp"
// #include "src/stl/bits/no_min_max.h"

template <typename axis_t, typename value_t>
struct table2D_lookup_cache 
{
  uint8_t lastXMax = 1;

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
    axis_t m = X - xMinValue;
    axis_t n = xMaxValue - xMinValue;

    value_t yMax = fromTable->values[fromTable->cache.lastXMax];
    value_t yMin = fromTable->values[fromTable->cache.lastXMax - 1];

    /* Float version (if m, yMax, yMin and n were float's)
       int yVal = (m * (yMax - yMin)) / n;
    */
    
    //Non-Float version
    int16_t yVal = ( ((int32_t) m) * (yMax-yMin) ) / n;
    return yMin + yVal;
}

template <typename axis_t, typename value_t, uint8_t sizeT>
value_t table2D_getValue(table2D<axis_t, value_t, sizeT> *fromTable, axis_t X_in)
{
  //Check whether the X input is the same as last time this ran
  if( (X_in == fromTable->cache.lastInput) && (fromTable->cache.cacheTime == getCacheTime()) )
  {
    // No-op
  }
  //If the requested X value is greater/small than the maximum/minimum bin, simply return that value
  else if(X_in >= fromTable->axisX[sizeT-1])
  {
    fromTable->cache.lastOutput = fromTable->values[sizeT-1];
  }
  else if(X_in <= fromTable->axisX[0])
  {
    fromTable->cache.lastOutput = fromTable->values[0];
  }
  //Finally if none of that is found
  else
  {
    //1st check is whether we're still in the same X bin as last time
    axis_t xMaxValue = fromTable->axisX[fromTable->cache.lastXMax];
    axis_t xMinValue = fromTable->axisX[fromTable->cache.lastXMax-1];
    if ( (X_in <= xMaxValue) && (X_in > xMinValue) )
    {
      fromTable->cache.lastOutput = interpolate(fromTable, X_in, xMaxValue, xMinValue);
    }
    else
    {
      //If we're not in the same bin, loop through to find where we are
      xMaxValue = fromTable->axisX[sizeT-1]; // init xMaxValue in preparation for loop.
      for (fromTable->cache.lastXMax = sizeT-1; fromTable->cache.lastXMax > 0; --(fromTable->cache.lastXMax))
      {
        //Checks the case where the X value is exactly what was requested
        if (X_in == xMaxValue)
        {
          fromTable->cache.lastOutput = fromTable->values[fromTable->cache.lastXMax]; //Simply return the coresponding value
          break;
        }
        else
        {
          xMinValue = fromTable->axisX[fromTable->cache.lastXMax-1]; // fetch next Min
          if (X_in > xMinValue)
          {
            // Value is in the current bin
            fromTable->cache.lastOutput = interpolate(fromTable, X_in, xMaxValue, xMinValue);
            break;
          }
          // Otherwise, continue to next bin
          xMaxValue = xMinValue; // for the next bin, our Min is their Max
        }
      }
    }
  } //X_in same as last time

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