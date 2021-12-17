/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/*
Because the size of the table is dynamic, this function is required to reallocate the array sizes
Note that this may clear some of the existing values of the table
*/
#include "table2d.h"
#if !defined(UNIT_TEST)
#include "globals.h"
#endif

/**
 * @brief Returns an axis (bin) value from the 2D table. This works regardless of whether that axis is bytes or int16_ts
 * 
 * @param fromTable 
 * @param X_in 
 * @return int16_t 
 */
int16_t table2D_getAxisValue(const struct table2D *fromTable, uint8_t X_in)
{
  int returnValue = 0;

  if(fromTable->axisSize == SIZE_INT) { returnValue = ((int16_t*)fromTable->axisX)[X_in]; }
  else if(fromTable->axisSize == SIZE_BYTE) { returnValue = ((uint8_t*)fromTable->axisX)[X_in]; }

  return returnValue;
}

/**
 * @brief Returns an value from the 2D table given an index value. No interpolation is performed
 * 
 * @param fromTable 
 * @param X_index 
 * @return int16_t 
 */
int16_t table2D_getRawValue(const struct table2D *fromTable, uint8_t X_index)
{
  int returnValue = 0;

  if(fromTable->valueSize == SIZE_INT) { returnValue = ((int16_t*)fromTable->values)[X_index]; }
  else if(fromTable->valueSize == SIZE_BYTE) { returnValue = ((uint8_t*)fromTable->values)[X_index]; }

  return returnValue;
}

static inline uint8_t getCacheTime(void) {
#if !defined(UNIT_TEST)
  return currentStatus.secl;
#else
  return 0;
#endif
}

static inline int16_t interpolate(const table2D *fromTable, const int16_t &X, const int16_t &xMaxValue, const int16_t xMinValue)
{
    int16_t m = X - xMinValue;
    int16_t n = xMaxValue - xMinValue;

    int16_t yMax = table2D_getRawValue(fromTable, fromTable->lastXMax);
    int16_t yMin = table2D_getRawValue(fromTable, fromTable->lastXMax - 1);

    /* Float version (if m, yMax, yMin and n were float's)
       int yVal = (m * (yMax - yMin)) / n;
    */
    
    //Non-Float version
    int16_t yVal = ( ((int32_t) m) * (yMax-yMin) ) / n;
    return yMin + yVal;
}

/*
This function pulls a 1D linear interpolated (ie averaged) value from a 2D table
ie: Given a value on the X axis, it returns a Y value that corresponds to the point on the curve between the nearest two defined X values

This function must take into account whether a table contains 8-bit or 16-bit values.
Unfortunately this means many of the lines are duplicated depending on this
*/
int table2D_getValue(struct table2D *fromTable, int X_in)
{
  //Check whether the X input is the same as last time this ran
  if( (X_in == fromTable->lastInput) && (fromTable->cacheTime == getCacheTime()) )
  {
    // No-op
  }
  //If the requested X value is greater/small than the maximum/minimum bin, simply return that value
  else if(X_in >= table2D_getAxisValue(fromTable, fromTable->xSize-1))
  {
    fromTable->lastOutput = table2D_getRawValue(fromTable, fromTable->xSize-1);
  }
  else if(X_in <= table2D_getAxisValue(fromTable, 0))
  {
    fromTable->lastOutput = table2D_getRawValue(fromTable, 0);
  }
  //Finally if none of that is found
  else
  {
    //1st check is whether we're still in the same X bin as last time
    int16_t xMaxValue = table2D_getAxisValue(fromTable, fromTable->lastXMax);
    int16_t xMinValue = table2D_getAxisValue(fromTable, fromTable->lastXMax-1);
    if ( (X_in <= xMaxValue) && (X_in > xMinValue) )
    {
      fromTable->lastOutput = interpolate(fromTable, X_in, xMaxValue, xMinValue);
    }
    else
    {
      //If we're not in the same bin, loop through to find where we are
      xMaxValue = table2D_getAxisValue(fromTable, fromTable->xSize-1); // init xMaxValue in preparation for loop.
      for (fromTable->lastXMax = fromTable->xSize-1; fromTable->lastXMax > 0; --(fromTable->lastXMax))
      {
        //Checks the case where the X value is exactly what was requested
        if (X_in == xMaxValue)
        {
          fromTable->lastOutput = table2D_getRawValue(fromTable, fromTable->lastXMax); //Simply return the coresponding value
          break;
        }
        else
        {
          xMinValue = table2D_getAxisValue(fromTable, fromTable->lastXMax-1); // fetch next Min
          if (X_in > xMinValue)
          {
            // Value is in the current bin
            fromTable->lastOutput = interpolate(fromTable, X_in, xMaxValue, xMinValue);
            break;
          }
          // Otherwise, continue to next bin
          xMaxValue = xMinValue; // for the next bin, our Min is their Max
        }
      }
    }
  } //X_in same as last time

  fromTable->cacheTime = getCacheTime(); //As we're not using the cache value, set the current secl value to track when this new value was calc'd
  fromTable->lastInput = X_in;
  return fromTable->lastOutput;
}
