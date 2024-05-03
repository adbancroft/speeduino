/*
This file is used for everything related to maps/tables including their definition, functions etc
*/
#ifndef TABLE_H
#define TABLE_H

#include "globals.h"
#include <stdint.h>

#define SIZE_SIGNED_BYTE    4U
#define SIZE_BYTE           8U
#define SIZE_INT            16U

/*
The 2D table can contain either 8-bit (byte) or 16-bit (int) values
The valueSize variable should be set to either 8 or 16 to indicate this BEFORE the table is used
*/
struct table2D {
  //Used 5414 RAM with original version
  uint8_t valueSize;
  uint8_t axisSize;
  uint8_t xSize;

  void *values;
  void *axisX;

  //Store the last X and Y coordinates in the table. This is used to make the next check faster
  mutable int16_t lastXMax;
  mutable int16_t lastXMin;

  //Store the last input and output for caching
  mutable int16_t lastInput;
  mutable int16_t lastOutput;
  mutable byte cacheTime; //Tracks when the last cache value was set so it can expire after x seconds. A timeout is required to pickup when a tuning value is changed, otherwise the old cached value will continue to be returned as the X value isn't changing. 
};

void construct2dTable(table2D &table, uint8_t length, uint8_t *values, uint8_t *bins);
void construct2dTable(table2D &table, uint8_t length, uint8_t *values, int8_t *bins);
void construct2dTable(table2D &table, uint8_t length, uint16_t *values, uint16_t *bins);
void construct2dTable(table2D &table, uint8_t length, uint8_t *values, uint16_t *bins);
void construct2dTable(table2D &table, uint8_t length, uint16_t *values, uint8_t *bins);
void construct2dTable(table2D &table, uint8_t length, int16_t *values, uint8_t *bins);

int16_t table2D_getAxisValue(const struct table2D *fromTable, byte X_in);
int16_t table2D_getRawValue(const struct table2D *fromTable, byte X_index);

int table2D_getValue(const struct table2D *fromTable, int X_in);

#endif // TABLE_H
