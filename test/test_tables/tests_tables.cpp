#include <unity.h>
#include <stdio.h>
#include "tests_tables.h"
#include "table3d.h"
#include "test_utils.h"

#define _countof(x) (sizeof(x) / sizeof (x[0]))

#if defined(ARDUINO)
#include <avr/pgmspace.h>
#else
#define PROGMEM
#endif

const PROGMEM uint8_t values[] = {
 //0    1    2   3     4    5    6    7    8    9   10   11   12   13    14   15
34,  34,  34,  34,  34,  34,  34,  34,  34,  35,  35,  35,  35,  35,  35,  35, 
34,  35,  36,  37,  39,  41,  42,  43,  43,  44,  44,  44,  44,  44,  44,  44, 
35,  36,  38,  41,  44,  46,  47,  48,  48,  49,  49,  49,  49,  49,  49,  49, 
36,  39,  42,  46,  50,  51,  52,  53,  53,  53,  53,  53,  53,  53,  53,  53, 
38,  43,  48,  52,  55,  56,  57,  58,  58,  58,  58,  58,  58,  58,  58,  58, 
42,  49,  54,  58,  61,  62,  62,  63,  63,  63,  63,  63,  63,  63,  63,  63, 
48,  56,  60,  64,  66,  66,  68,  68,  68,  68,  68,  68,  68,  68,  68,  68, 
54,  62,  66,  69,  71,  71,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72, 
61,  69,  72,  74,  76,  76,  77,  77,  77,  77,  77,  77,  77,  77,  77,  77, 
68,  75,  78,  79,  81,  81,  81,  82,  82,  82,  82,  82,  82,  82,  82,  82, 
74,  80,  83,  84,  85,  86,  86,  86,  87,  87,  87,  87,  87,  87,  87,  87, 
81,  86,  88,  89,  90,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91, 
93,  96,  98,  99,  99,  100, 100, 101, 101, 101, 101, 101, 101, 101, 101, 101, 
98,  101, 103, 103, 104, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 
104, 106, 107, 108, 109, 109, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 
109, 111, 112, 113, 114, 114, 114, 115, 115, 115, 114, 114, 114, 114, 114, 114, 
  };
static const table3d_axis_t tempXAxis[] = {500, 700, 900, 1200, 1600, 2000, 2500, 3100, 3500, 4100, 4700, 5300, 5900, 6500, 6750, 7000};
static const table3d_axis_t xMin = tempXAxis[0];
static const table3d_axis_t xMax = tempXAxis[_countof(tempXAxis)-1];
static const table3d_axis_t tempYAxis[] = { 16, 26, 30, 36, 40, 46, 50, 56, 60, 66, 70, 76, 86, 90, 96, 100};
static const table3d_axis_t yMin = tempYAxis[0];
static const table3d_axis_t yMax = tempYAxis[_countof(tempYAxis)-1];


static table3d16RpmLoad testTable;

void setup_TestTable(void)
{
  //Setup the fuel table with some sane values for testing
  //Table is setup per the below
  /*
  100 |  109 |  111 |  112 |  113 |  114 |  114 |  114 |  115 |  115 |  115 |  114 |  114 |  113 |  112 |  111 |  111
  96  |  104 |  106 |  107 |  108 |  109 |  109 |  110 |  110 |  110 |  110 |  110 |  109 |  108 |  107 |  107 |  106
  90  |   98 |  101 |  103 |  103 |  104 |  105 |  105 |  105 |  105 |  105 |  105 |  104 |  104 |  103 |  102 |  102
  86  |   93 |   96 |   98 |   99 |   99 |  100 |  100 |  101 |  101 |  101 |  100 |  100 |   99 |   98 |   98 |   97
  76  |   81 |   86 |   88 |   89 |   90 |   91 |   91 |   91 |   91 |   91 |   91 |   90 |   90 |   89 |   89 |   88
  70  |   74 |   80 |   83 |   84 |   85 |   86 |   86 |   86 |   87 |   86 |   86 |   86 |   85 |   84 |   84 |   84
  65  |   68 |   75 |   78 |   79 |   81 |   81 |   81 |   82 |   82 |   82 |   82 |   81 |   81 |   80 |   79 |   79
  60  |   61 |   69 |   72 |   74 |   76 |   76 |   77 |   77 |   77 |   77 |   77 |   76 |   76 |   75 |   75 |   74
  56  |   54 |   62 |   66 |   69 |   71 |   71 |   72 |   72 |   72 |   72 |   72 |   72 |   71 |   71 |   70 |   70
  50  |   48 |   56 |   60 |   64 |   66 |   66 |   68 |   68 |   68 |   68 |   67 |   67 |   67 |   66 |   66 |   65
  46  |   42 |   49 |   54 |   58 |   61 |   62 |   62 |   63 |   63 |   63 |   63 |   62 |   62 |   61 |   61 |   61
  40  |   38 |   43 |   48 |   52 |   55 |   56 |   57 |   58 |   58 |   58 |   58 |   58 |   57 |   57 |   57 |   56
  36  |   36 |   39 |   42 |   46 |   50 |   51 |   52 |   53 |   53 |   53 |   53 |   53 |   53 |   52 |   52 |   52
  30  |   35 |   36 |   38 |   41 |   44 |   46 |   47 |   48 |   48 |   49 |   48 |   48 |   48 |   48 |   47 |   47
  26  |   34 |   35 |   36 |   37 |   39 |   41 |   42 |   43 |   43 |   44 |   44 |   44 |   43 |   43 |   43 |   43
  16  |   34 |   34 |   34 |   34 |   34 |   34 |   34 |   34 |   34 |   35 |   34 |   34 |   34 |   34 |   34 |   34
      ----------------------------------------------------------------------------------------------------------------
         500 |  700 |  900 | 1200 | 1600 | 2000 | 2500 | 3100 | 3500 | 4100 | 4700 | 5300 | 5900 | 6500 | 6750 | 7000
  */

  //
  // NOTE: USE OF ITERATORS HERE IS DELIBERATE. IT INCLUDES THEM IN THE UNIT TESTS, giving
  // them some coverage
  //
  {
    table_axis_iterator itX = testTable.axisX.begin();
    const table3d_axis_t *pXValue = tempXAxis;
    while (!itX.at_end())
    {
      *itX = *pXValue;
      ++pXValue;
      ++itX;
    }
  }
  {
    table_axis_iterator itY = testTable.axisY.begin();
    const table3d_axis_t *pYValue = tempYAxis;
    while (!itY.at_end())
    {
      *itY = *pYValue;
      ++pYValue;
      ++itY;
    }
  }

  {
    table_value_iterator itZ = testTable.values.begin();
    const table3d_value_t *pZValue = values;
    while (!itZ.at_end())
    {
      table_row_iterator itRow = *itZ;
      while (!itRow.at_end())
      {
#if defined(ARDUINO)
        *itRow = pgm_read_byte(pZValue);
#else
        *itRow = *pZValue;
#endif
        ++pZValue;
        ++itRow;
      }
      ++itZ;
    }
  }
}

void test_tableLookup_50_50(void)
{
  //Tests a lookup that is exactly 50% of the way between cells on both the X and Y axis
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, intermediate(tempYAxis[6], tempYAxis[7], 50), 
                                      intermediate(tempXAxis[5], tempXAxis[6], 50));
  TEST_ASSERT_EQUAL_UINT8(69, tempVE);
  TEST_ASSERT_EQUAL_UINT8(9, testTable.get_value_cache.lastXBinMax);
  TEST_ASSERT_EQUAL_UINT8(8, testTable.get_value_cache.lastYBinMax);
}

void test_tableLookup_33_33(void)
{
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, intermediate(tempYAxis[6], tempYAxis[7], 33), 
                                      intermediate(tempXAxis[5], tempXAxis[6], 33));
  TEST_ASSERT_EQUAL_UINT8(67, tempVE);
  TEST_ASSERT_EQUAL_UINT8(9, testTable.get_value_cache.lastXBinMax);
  TEST_ASSERT_EQUAL_UINT8(8, testTable.get_value_cache.lastYBinMax);
}

void test_tableLookup_33_66(void)
{
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, intermediate(tempYAxis[6], tempYAxis[7], 33), 
                                      intermediate(tempXAxis[5], tempXAxis[6], 66));
  TEST_ASSERT_EQUAL_UINT8(68, tempVE);
  TEST_ASSERT_EQUAL_UINT8(9, testTable.get_value_cache.lastXBinMax);
  TEST_ASSERT_EQUAL_UINT8(8, testTable.get_value_cache.lastYBinMax);
}

void test_tableLookup_66_66(void)
{
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, intermediate(tempYAxis[6], tempYAxis[7], 66), 
                                      intermediate(tempXAxis[5], tempXAxis[6], 66));
  TEST_ASSERT_EQUAL_UINT8(69, tempVE);
  TEST_ASSERT_EQUAL_UINT8(9, testTable.get_value_cache.lastXBinMax);
  TEST_ASSERT_EQUAL_UINT8(8, testTable.get_value_cache.lastYBinMax);
}

void test_tableLookup_66_33(void)
{
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, intermediate(tempYAxis[6], tempYAxis[7], 66), 
                                      intermediate(tempXAxis[5], tempXAxis[6], 33));
  TEST_ASSERT_EQUAL_UINT8(68, tempVE);
  TEST_ASSERT_EQUAL_UINT8(9, testTable.get_value_cache.lastXBinMax);
  TEST_ASSERT_EQUAL_UINT8(8, testTable.get_value_cache.lastYBinMax);
}

void test_tableLookup_exact1Axis(void)
{
  //Tests a lookup that exactly matches on the X axis and 50% of the way between cells on the Y axis
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, intermediate(tempYAxis[5], tempYAxis[6], 50), testTable.axisX.axis[6]);
  TEST_ASSERT_EQUAL_UINT8(65, tempVE);
  TEST_ASSERT_EQUAL_UINT8(6, testTable.get_value_cache.lastXBinMax);
  TEST_ASSERT_EQUAL_UINT8(9, testTable.get_value_cache.lastYBinMax);
}

void test_tableLookup_exact2Axis(void)
{
  //Tests a lookup that exactly matches on both the X and Y axis
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, testTable.axisY.axis[5], testTable.axisX.axis[9]);
  TEST_ASSERT_EQUAL_UINT8(86, tempVE);
  TEST_ASSERT_EQUAL_UINT8(9, testTable.get_value_cache.lastXBinMax);
  TEST_ASSERT_EQUAL_UINT8(5, testTable.get_value_cache.lastYBinMax);
}

void test_tableLookup_overMaxX(void)
{
  //Tests a lookup where the RPM exceeds the highest value in the table. The Y value is a 50% match
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, intermediate(tempYAxis[10], tempYAxis[11], 50), xMax+100);
  TEST_ASSERT_EQUAL_UINT8(89, tempVE);
  TEST_ASSERT_EQUAL_UINT8(0, testTable.get_value_cache.lastXBinMax);
  TEST_ASSERT_EQUAL_UINT8(4, testTable.get_value_cache.lastYBinMax);
}

void test_tableLookup_overMaxY(void)
{
  //Tests a lookup where the load value exceeds the highest value in the table. The X value is a 50% match
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, yMax+10, intermediate(tempXAxis[0], tempXAxis[1], 50));
  TEST_ASSERT_EQUAL_UINT8(110, tempVE);
  TEST_ASSERT_EQUAL_UINT8(14, testTable.get_value_cache.lastXBinMax);
  TEST_ASSERT_EQUAL_UINT8(0, testTable.get_value_cache.lastYBinMax);
}

void test_tableLookup_underMinX(void)
{
  //Tests a lookup where the RPM value is below the lowest value in the table. The Y value is a 50% match
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, intermediate(tempYAxis[3], tempYAxis[4], 50), xMin-100);
  TEST_ASSERT_EQUAL_UINT8(37, tempVE);
  TEST_ASSERT_EQUAL_UINT8(14, testTable.get_value_cache.lastXBinMax);
  TEST_ASSERT_EQUAL_UINT8(11, testTable.get_value_cache.lastYBinMax);
}

void test_tableLookup_underMinY(void)
{
  //Tests a lookup where the load value is below the lowest value in the table. The X value is a 50% match
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, yMin-5, intermediate(tempXAxis[0], tempXAxis[1], 50));
  TEST_ASSERT_EQUAL_UINT8(34, tempVE);
  TEST_ASSERT_EQUAL_UINT8(14, testTable.get_value_cache.lastXBinMax);
  TEST_ASSERT_EQUAL_UINT8(14, testTable.get_value_cache.lastYBinMax);
}

void test_tableLookup_roundUp(void)
{
  // Tests a lookup where the inputs result in a value that is outside the table range
  // due to fixed point rounding
  // Issue #726
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, 17, 600);
  TEST_ASSERT_EQUAL_UINT8(34, tempVE);
  TEST_ASSERT_EQUAL_UINT8(14, testTable.get_value_cache.lastXBinMax);
  TEST_ASSERT_EQUAL_UINT8(14, testTable.get_value_cache.lastYBinMax);
}

void test_all_incrementing(void)
{
  //Test the when going up both the load and RPM axis that the returned value is always equal or higher to the previous one
  //Tests all combinations of load/rpm from between 0-200 load and 0-9000 rpm
  //WARNING: This can take a LONG time to run. It is disabled by default for this reason
  uint16_t tempVE = 0;
  
  for(uint16_t rpm = 0; rpm<xMax+1000; rpm+=100)
  {
    tempVE = 0;
    for(uint8_t load = 0; load<yMax+10; load++)
    {
      uint16_t newVE = get3DTableValue(&testTable, load, rpm);
      // char buffer[256];
      // sprintf(buffer, "%d, %d"
      //                 ", %d, %d, %d, %d"
      //                 ", %d, %d, %d, %d"
      //                 ", %d", 
      //                 rpm, load, 
      //                 testTable.get_value_cache.lastXMin, testTable.get_value_cache.lastXBinMax,
      //                 tempXAxis[testTable.get_value_cache.lastXMin], tempXAxis[testTable.get_value_cache.lastXBinMax],

      //                 testTable.get_value_cache.lastYMin, testTable.get_value_cache.lastYBinMax,
      //                 tempYAxis[testTable.get_value_cache.lastYMin], tempYAxis[testTable.get_value_cache.lastYBinMax],

      //                 newVE);
      // TEST_MESSAGE(buffer);
      TEST_ASSERT_GREATER_OR_EQUAL(tempVE, newVE);
      tempVE = newVE;
    }
  }
}

void testTables()
{
  RUN_TEST(test_tableLookup_50_50);
  RUN_TEST(test_tableLookup_33_33);
  RUN_TEST(test_tableLookup_33_66);
  RUN_TEST(test_tableLookup_66_66);
  RUN_TEST(test_tableLookup_66_33);
  RUN_TEST(test_tableLookup_exact1Axis);
  RUN_TEST(test_tableLookup_exact2Axis);
  RUN_TEST(test_tableLookup_overMaxX);
  RUN_TEST(test_tableLookup_overMaxY);
  RUN_TEST(test_tableLookup_underMinX);
  RUN_TEST(test_tableLookup_underMinY);
  RUN_TEST(test_tableLookup_roundUp);
#if !defined(ARDUINO)
  RUN_TEST(test_all_incrementing);
#endif
}