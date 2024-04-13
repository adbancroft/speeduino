
#pragma once

#include <Arduino.h>

// Unity macro to reduce memory usage (RAM, .bss)
//
// Unity supplied RUN_TEST captures the function name
// using #func directly in the call to UnityDefaultTestRun.
// This is a raw string that is placed in the data segment,
// which consumes RAM.
//
// So instead, place the function name in flash memory and
// load it at run time.
#define RUN_TEST_P(func) \
  { \
    char funcName[128]; \
    strcpy_P(funcName, PSTR(#func)); \
    UnityDefaultTestRun(func, funcName, __LINE__); \
  }

#if !defined(_countof)
#define _countof(x) (sizeof(x) / sizeof (x[0]))
#endif

#if defined(PROGMEM)
#define TEST_DATA_P static constexpr PROGMEM
#else
#define TEST_DATA_P static constexpr
#endif

template <typename table3d_t>
static inline void populate_table_P(table3d_t &table, 
                                  const table3d_axis_t *pXValues,   // PROGMEM if available
                                  const table3d_axis_t *pYValues,   // PROGMEM if available
                                  const table3d_value_t *pZValues)  // PROGMEM if available
{
  {
    table_axis_iterator itX = table.axisX.begin();
    while (!itX.at_end())
    {
#if defined(PROGMEM)
      *itX = (table3d_axis_t)pgm_read_word(pXValues);
#else
      *itX = *pXValues;
#endif      
      ++pXValues;
      ++itX;
    }
  }  
  {
    table_axis_iterator itY = table.axisY.begin();
    while (!itY.at_end())
    {
#if defined(PROGMEM)
      *itY = (table3d_axis_t)pgm_read_word(pYValues);
#else
      *itY = *pYValues;
#endif      
      ++pYValues;
      ++itY;
    }
  }
  {
    table_value_iterator itZ = table.values.begin();
    while (!itZ.at_end())
    {
      table_row_iterator itRow = *itZ;
      while (!itRow.at_end())
      {
#if defined(PROGMEM)
        *itRow = pgm_read_byte(pZValues);
#else
        *itRow = *pZValues;
#endif
        ++pZValues;
        ++itRow;
      }
      ++itZ;
    }
  }
}