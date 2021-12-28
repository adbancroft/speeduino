#include <unity.h>
#include "tests_tables.h"
#include "test_table2d.h"
#include "test_find_bin.h"
#include "test_saturated_cast.h"
#include "test_muldiv.h"

void run_tests(void)
{
    UNITY_BEGIN();    // IMPORTANT LINE!

    testTables();
    testTable2d();
    testFindBin();
    test_saturated_cast();
    testmuldiv();

    UNITY_END(); // stop unit testing    
}

#if defined(ARDUINO)
#include <Arduino.h>
// Device build
void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    run_tests();
}

void loop()
{
    // Blink to indicate end of test
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
}
#else

#include "table3d_interpolate.cpp"

// Native build
int main(int argc, char **argv) {
  run_tests();
  return 0;
}
#endif