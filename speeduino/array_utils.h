#pragma once

#include <stdint.h>

template <typename T>
inline bool is_in_bin(const T &testValue, const T &min, const T &max)
{
  return testValue > min && testValue <= max;
}

// Find the axis index for the top of the bin that covers the test value.
// E.g. 4 in { 1, 3, 5, 7, 9 } would be 2
// We assume the axis is in order.
template <typename T>
inline uint8_t find_bin(
  T &value,               // Value to search for
  const T *pAxis,         // The axis to search
  uint8_t minElement,     // Axis index of the element with the lowest value (at one end of the array)
  uint8_t maxElement,     // Axis index of the element with the highest value (at the other end of the array)
  uint8_t lastBinMax)     // The last result from this call - used to speed up searches
{
  // Direction to search (1 coventional, -1 to go backwards from pAxis)
  int8_t stride = maxElement>minElement ? 1 : -1;
  // It's quicker to increment/adjust this pointer than to repeatedly 
  // index the array - minimum 2%, often >5%
  const T *pMax = nullptr;
  // minElement is at one end of the array, so the "lowest" bin 
  // is [minElement, minElement+stride]. Since we're working with the upper
  // index of the bin pair, we can't go below minElement + stride.
  uint8_t minBinIndex = minElement + stride;

  // Check the cached last bin and either side first - it's likely that this will give a hit under
  // real world conditions

  // Check if we're still in the same bin as last time
  pMax = pAxis + lastBinMax;
  if (is_in_bin(value, *(pMax - stride), *pMax))
  {
    return lastBinMax;
  }
  // Check the bin above the last one
  pMax = pMax - stride;
  if (lastBinMax!=minBinIndex && is_in_bin(value, *(pMax - stride), *pMax))
  {
    return lastBinMax-stride;    
  }
  // Check the bin below the last one
  pMax += stride*2;
  if (lastBinMax!=maxElement && is_in_bin(value, *(pMax - stride), *pMax))
  {
    return lastBinMax+stride;
  }

  // Check if outside array limits - won't happen often in the real world
  // so check after the cache check
  // At or above maximum - clamp to final value
  if (value>=pAxis[maxElement])
  {
    value = pAxis[maxElement];
    return maxElement;
  }
  // At or below minimum - clamp to lowest value
  if (value<=pAxis[minElement])
  {
    value = pAxis[minElement];
    return minElement+stride;
  }

  // No hits above, so run a linear search.
  // We start at the maximum & work down, rather than looping from [0] up to [max]
  // This is because the important tables (fuel and injection) will have the highest
  // RPM at the top of the X axis, so starting there will mean the best case occurs 
  // when the RPM is highest (and hence the CPU is needed most)
  lastBinMax = maxElement;
  pMax = pAxis + lastBinMax;
  while (lastBinMax!=minBinIndex && !is_in_bin(value, *(pMax - stride), *pMax))
  {
    lastBinMax -= stride;
    pMax -= stride;
  }
  return lastBinMax;
}
