
#pragma once

#include <Arduino.h>
#include <unity.h>
#include "table2d.h"
#include "table3d.h"

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

static inline void populate_2dtable(table2D *pTable, uint8_t value, uint8_t bin) {
  for (uint8_t index=0; index<pTable->xSize; ++index) {
    ((uint8_t*)pTable->values)[index] = value;
    ((uint8_t*)pTable->axisX)[index] = bin;
  }
  pTable->cacheTime = UINT8_MAX;
}

static inline size_t getTableElementSize(uint8_t type) {
  switch (type)
  {
    case SIZE_INT: return sizeof(int16_t);
    case SIZE_BYTE: return sizeof(uint8_t);
    case SIZE_SIGNED_BYTE: return sizeof(int8_t);
    default: TEST_ABORT();
  }
}

static inline size_t getAxisElementSize(const table2D *pTable) {
  return getTableElementSize(pTable->axisSize);
}

static inline size_t getValueElementSize(const table2D *pTable) {
  return getTableElementSize(pTable->valueSize);
}

static inline void populate_2dtable(table2D *pTable, const uint8_t values[], const uint8_t bins[]) {
  memcpy(pTable->axisX, bins, getAxisElementSize(pTable)*pTable->xSize);
  memcpy(pTable->values, values, getValueElementSize(pTable)*pTable->xSize);
  pTable->cacheTime = UINT8_MAX;
}

static inline void populate_2dtable_P(table2D *pTable, const uint8_t values[], const uint8_t bins[]) {
  memcpy_P(pTable->axisX, bins, getAxisElementSize(pTable)*pTable->xSize);
  memcpy_P(pTable->values, values, getValueElementSize(pTable)*pTable->xSize);
  pTable->cacheTime = UINT8_MAX;
}
