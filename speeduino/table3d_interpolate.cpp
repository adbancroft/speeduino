#include "table3d_interpolate.h"
#include "find_bin.h"
#include "math/muldiv.h"
#include "math/FixedPoint.h"
#include <type_traits>

// ============================= Axis value to bin % =========================

static inline QU1X8_t compute_bin_position(table3d_axis_t value, const bin<table3d_axis_t> &bin)
{
  if (value<=bin.min) { return 0u; }
  if (value>=bin.max) { return QU1X8_ONE; }

  // Since the axis values are in increasing order, we can use unsigned math
  typedef typename std::make_unsigned<table3d_axis_t>::type utable3d_axis_t;

  utable3d_axis_t binWidth = bin.max - bin.min;
  utable3d_axis_t binDistance = value - bin.min;
  return (QU1X8_t)muldiv(binDistance, QU1X8_ONE, binWidth); 
}


// ============================= End internal support functions =========================

//This function pulls a value from a 3D table given a target for X and Y coordinates.
//It performs a 2D linear interpolation as described in: www.megamanual.com/v22manual/ve_tuner.pdf
table3d_value_t __attribute__((noclone)) get3DTableValue(struct table3DGetValueCache *pValueCache, 
                    table3d_dim_t axisSize,
                    const table3d_value_t *pValues,
                    const table3d_axis_t *pXAxis,
                    const table3d_axis_t *pYAxis,
                    table3d_axis_t Y_in, table3d_axis_t X_in)
{
    //0th check is whether the same X and Y values are being sent as last time. 
    // If they are, this not only prevents a lookup of the axis, but prevents the 
    //interpolation calcs being performed
    if( X_in == pValueCache->last_lookup.x && 
        Y_in == pValueCache->last_lookup.y)
    {
      return pValueCache->lastOutput;
    }

    // Assign this here, as we might modify coords below.
    pValueCache->last_lookup.x = X_in;
    pValueCache->last_lookup.y = Y_in;

    // Figure out where on the axes the incoming coord are
    bin<table3d_axis_t> xBin = find_bin(X_in, pXAxis, axisSize-1, 0, pValueCache->lastXBinMax);
    pValueCache->lastXBinMax = xBin.maxIdx;
    bin<table3d_axis_t> yBin = find_bin(Y_in, pYAxis, axisSize-1, 0, pValueCache->lastYBinMax);
    pValueCache->lastYBinMax = yBin.maxIdx;

    /*
    At this point we have the 4 corners of the map where the interpolated value will fall in
    Eg: (yMax,xMin)  (yMax,xMax)

        (yMin,xMin)  (yMin,xMax)

    In the following calculation the table values are referred to by the following variables:
              A          B

              C          D
    */
    table3d_dim_t rowMax = pValueCache->lastYBinMax * axisSize;
    table3d_dim_t rowMin = rowMax + axisSize;
    table3d_dim_t colMax = axisSize - pValueCache->lastXBinMax - 1U;
    table3d_dim_t colMin = colMax - 1U;
    table3d_value_t A = pValues[rowMax + colMin];
    table3d_value_t B = pValues[rowMax + colMax];
    table3d_value_t C = pValues[rowMin + colMin];
    table3d_value_t D = pValues[rowMin + colMax];

    //Check that all values aren't just the same (This regularly happens with things like the fuel trim maps)
    if( (A == B) && (A == C) && (A == D) ) { pValueCache->lastOutput = A; }
    else
    {
      //Create some normalised position values
      //These are essentially percentages (between 0 and 1) of where the desired value falls between the nearest bins on each axis
      const QU1X8_t p = compute_bin_position(X_in, xBin);
      const QU1X8_t q = compute_bin_position(Y_in, yBin);

      const QU1X8_t m = mulQU1X8(QU1X8_ONE-p, q);
      const QU1X8_t n = mulQU1X8(p, q);
      const QU1X8_t o = mulQU1X8(QU1X8_ONE-p, QU1X8_ONE-q);
      const QU1X8_t r = mulQU1X8(p, QU1X8_ONE-q);

      // The math below only works if the table values are 8 bit
      // and the fixed point math use 8 (or less) bits for storing
      // it's fractional part.
      //
      // If not we will hit integer overflow.
      static_assert(sizeof(decltype(A))==1 && QU1X8_INTEGER_SHIFT<=8, "get3DTableValue: table values must be 8 bit");
      pValueCache->lastOutput = ( (A * m) + (B * n) + (C * o) + (D * r) ) >> QU1X8_INTEGER_SHIFT;
    }

    return pValueCache->lastOutput;
}
