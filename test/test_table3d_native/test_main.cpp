#include <unity.h>
#include "table3d_interpolate.cpp"
typedef uint8_t byte;
#include "table2d.h"
#include "..\test_misc\tests_tables.cpp"
#include "..\test_misc\test_table2d.cpp"
#include "..\test_misc\test_find_bin.cpp"

int main(int argc, char **argv) {
  UNITY_BEGIN();
  
  testTables();
  RUN_TEST(test_all_incrementing);

  testTable2d();

  testFindBin();

  UNITY_END();

    return 0;
}