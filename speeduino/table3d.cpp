#include <Arduino.h>
#include "table3d.h"

//The shift amount used for the 3D table calculations
#define TABLE_SHIFT_FACTOR  8
#define TABLE_SHIFT_POWER   (1UL<<TABLE_SHIFT_FACTOR)

//This function pulls a value from a 3D table given a target for X and Y coordinates.
//It performs a 2D linear interpolation as descibred in: www.megamanual.com/v22manual/ve_tuner.pdf
table3d_value_t get3DTableValue(struct table3DGetValueCache *pValueCache, 
                    table3d_dim_t axisSize,
                    const table3d_value_t *pValues,
                    const table3d_axis_t *pXAxis,
                    const table3d_axis_t *pYAxis,
                    table3d_axis_t Y_in, table3d_axis_t X_in)
  {
    table3d_axis_t X = X_in;
    table3d_axis_t Y = Y_in;

    table3d_value_t tableResult = 0;
    //Loop through the X axis bins for the min/max pair
    //Note: For the X axis specifically, rather than looping from tableAxisX[0] up to tableAxisX[max], we start at tableAxisX[Max] and go down.
    //      This is because the important tables (fuel and injection) will have the highest RPM at the top of the X axis, so starting there will mean the best case occurs when the RPM is highest (And hence the CPU is needed most)
    table3d_axis_t xMinValue = pXAxis[0];
    table3d_axis_t xMaxValue = pXAxis[axisSize-1];
    byte xMin = 0;
    byte xMax = 0;

    //If the requested X value is greater/small than the maximum/minimum bin, reset X to be that value
    if(X > xMaxValue) { X = xMaxValue; }
    if(X < xMinValue) { X = xMinValue; }

    //0th check is whether the same X and Y values are being sent as last time. If they are, this not only prevents a lookup of the axis, but prevents the interpolation calcs being performed
    if( X_in == pValueCache->last_lookup.x && 
        Y_in == pValueCache->last_lookup.y)
    {
      return pValueCache->lastOutput;
    }

    //Commence the lookups on the X and Y axis

    //1st check is whether we're still in the same X bin as last time
    if ( (X <= pXAxis[pValueCache->lastXBins.max]) && (X > pXAxis[pValueCache->lastXBins.min]) )
    {
      xMaxValue = pXAxis[pValueCache->lastXBins.max];
      xMinValue = pXAxis[pValueCache->lastXBins.min];
      xMax = pValueCache->lastXBins.max;
      xMin = pValueCache->lastXBins.min;
    }
    //2nd check is whether we're in the next RPM bin (To the right)
    else if ( ((pValueCache->lastXBins.max + 1) < axisSize ) && (X <= pXAxis[pValueCache->lastXBins.max +1 ]) && (X > pXAxis[pValueCache->lastXBins.min + 1]) ) //First make sure we're not already at the last X bin
    {
      xMax = pValueCache->lastXBins.max + 1;
      pValueCache->lastXBins.max = xMax;
      xMin = pValueCache->lastXBins.min + 1;
      pValueCache->lastXBins.min = xMin;
      xMaxValue = pXAxis[pValueCache->lastXBins.max];
      xMinValue = pXAxis[pValueCache->lastXBins.min];
    }
    //3rd check is to look at the previous bin (to the left)
    else if ( (pValueCache->lastXBins.min > 0 ) && (X <= pXAxis[pValueCache->lastXBins.max - 1]) && (X > pXAxis[pValueCache->lastXBins.min - 1]) ) //First make sure we're not already at the first X bin
    {
      xMax = pValueCache->lastXBins.max - 1;
      pValueCache->lastXBins.max = xMax;
      xMin = pValueCache->lastXBins.min - 1;
      pValueCache->lastXBins.min = xMin;
      xMaxValue = pXAxis[pValueCache->lastXBins.max];
      xMinValue = pXAxis[pValueCache->lastXBins.min];
    }
    else
    //If it's not caught by one of the above scenarios, give up and just run the loop
    {
      for (int8_t x = axisSize-1; x >= 0; x--)
      {
        //Checks the case where the X value is exactly what was requested
        if ( (X == pXAxis[x]) || (x == 0) )
        {
          xMaxValue = pXAxis[x];
          xMinValue = pXAxis[x];
          xMax = x;
          pValueCache->lastXBins.max = xMax;
          xMin = x;
          pValueCache->lastXBins.min = xMin;
          break;
        }
        //Normal case
        if ( (X <= pXAxis[x]) && (X > pXAxis[x-1]) )
        {
          xMaxValue = pXAxis[x];
          xMinValue = pXAxis[x-1];
          xMax = x;
          pValueCache->lastXBins.max = xMax;
          xMin = x-1;
          pValueCache->lastXBins.min = xMin;
          break;
        }
      }
    }

    //Loop through the Y axis bins for the min/max pair
    table3d_axis_t yMaxValue = pYAxis[0];
    table3d_axis_t yMinValue = pYAxis[axisSize-1];
    byte yMin = 0;
    byte yMax = 0;

    //If the requested Y value is greater/small than the maximum/minimum bin, reset Y to be that value
    if(Y > yMaxValue) { Y = yMaxValue; }
    if(Y < yMinValue) { Y = yMinValue; }

    //1st check is whether we're still in the same Y bin as last time
    if ( (Y >= pYAxis[pValueCache->lastYBins.max]) && (Y < pYAxis[pValueCache->lastYBins.min]) )
    {
      yMaxValue = pYAxis[pValueCache->lastYBins.max];
      yMinValue = pYAxis[pValueCache->lastYBins.min];
      yMax = pValueCache->lastYBins.max;
      yMin = pValueCache->lastYBins.min;
    }
    //2nd check is whether we're in the next MAP/TPS bin (Next one up)
    else if ( (pValueCache->lastYBins.min > 0 ) && (Y <= pYAxis[pValueCache->lastYBins.min - 1 ]) && (Y > pYAxis[pValueCache->lastYBins.max - 1]) ) //First make sure we're not already at the top Y bin
    {
      yMax = pValueCache->lastYBins.max - 1;
      pValueCache->lastYBins.max = yMax;
      yMin = pValueCache->lastYBins.min - 1;
      pValueCache->lastYBins.min = yMin;
      yMaxValue = pYAxis[pValueCache->lastYBins.max];
      yMinValue = pYAxis[pValueCache->lastYBins.min];
    }
    //3rd check is to look at the previous bin (Next one down)
    else if ( ((pValueCache->lastYBins.max + 1) < axisSize) && (Y <= pYAxis[pValueCache->lastYBins.min + 1]) && (Y > pYAxis[pValueCache->lastYBins.max + 1]) ) //First make sure we're not already at the bottom Y bin
    {
      yMax = pValueCache->lastYBins.max + 1;
      pValueCache->lastYBins.max = yMax;
      yMin = pValueCache->lastYBins.min + 1;
      pValueCache->lastYBins.min = yMin;
      yMaxValue = pYAxis[pValueCache->lastYBins.max];
      yMinValue = pYAxis[pValueCache->lastYBins.min];
    }
    else
    //If it's not caught by one of the above scenarios, give up and just run the loop
    {

      for (int8_t y = axisSize-1; y >= 0; y--)
      {
        //Checks the case where the Y value is exactly what was requested
        if ( (Y == pYAxis[y]) || (y==0) )
        {
          yMaxValue = pYAxis[y];
          yMinValue = pYAxis[y];
          yMax = y;
          pValueCache->lastYBins.max = yMax;
          yMin = y;
          pValueCache->lastYBins.min = yMin;
          break;
        }
        //Normal case
        if ( (Y >= pYAxis[y]) && (Y < pYAxis[y-1]) )
        {
          yMaxValue = pYAxis[y];
          yMinValue = pYAxis[y-1];
          yMax = y;
          pValueCache->lastYBins.max = yMax;
          yMin = y-1;
          pValueCache->lastYBins.min = yMin;
          break;
        }
      }
    }


    /*
    At this point we have the 4 corners of the map where the interpolated value will fall in
    Eg: (yMin,xMin)  (yMin,xMax)

        (yMax,xMin)  (yMax,xMax)

    In the following calculation the table values are referred to by the following variables:
              A          B

              C          D

    */
    table3d_axis_t A = pValues[(yMin*axisSize)+xMin];
    table3d_axis_t B = pValues[(yMin*axisSize)+xMax];
    table3d_axis_t C = pValues[(yMax*axisSize)+xMin];
    table3d_axis_t D = pValues[(yMax*axisSize)+xMax];

    //Check that all values aren't just the same (This regularly happens with things like the fuel trim maps)
    if( (A == B) && (A == C) && (A == D) ) { tableResult = A; }
    else
    {
      //Create some normalised position values
      //These are essentially percentages (between 0 and 1) of where the desired value falls between the nearest bins on each axis


      //Initial check incase the values were hit straight on

      unsigned long p = (long)X - xMinValue;
      if (xMaxValue == xMinValue) { p = (p << TABLE_SHIFT_FACTOR); }  //This only occurs if the requested X value was equal to one of the X axis bins
      else { p = ( (p << TABLE_SHIFT_FACTOR) / (xMaxValue - xMinValue) ); } //This is the standard case

      unsigned long q;
      if (yMaxValue == yMinValue)
      {
        q = (long)Y - yMinValue;
        q = (q << TABLE_SHIFT_FACTOR);
      }
      //Standard case
      else
      {
        q = long(Y) - yMaxValue;
        q = TABLE_SHIFT_POWER - ( (q << TABLE_SHIFT_FACTOR) / (yMinValue - yMaxValue) );
      }

      uint32_t m = ((TABLE_SHIFT_POWER-p) * (TABLE_SHIFT_POWER-q)) >> TABLE_SHIFT_FACTOR;
      uint32_t n = (p * (TABLE_SHIFT_POWER-q)) >> TABLE_SHIFT_FACTOR;
      uint32_t o = ((TABLE_SHIFT_POWER-p) * q) >> TABLE_SHIFT_FACTOR;
      uint32_t r = (p * q) >> TABLE_SHIFT_FACTOR;
      tableResult = ( (A * m) + (B * n) + (C * o) + (D * r) ) >> TABLE_SHIFT_FACTOR;
    }

    //Update the tables cache data
    pValueCache->last_lookup.x = X_in;
    pValueCache->last_lookup.y = Y_in;
    pValueCache->lastOutput = tableResult;

    return tableResult;
}
