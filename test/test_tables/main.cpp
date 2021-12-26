#include <Arduino.h>
#include <unity.h>

#include "tests_tables.h"
#include "test_table2d.h"
// #include "tests_maths.h"
#include "test_find_bin.h"
#include "test_saturated_cast.h"

#define UNITY_EXCLUDE_DETAILS

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN();    // IMPORTANT LINE!

    testTables();
    testTable2d();
    // testMaths();
    testFindBin();
    test_saturated_cast();

    UNITY_END(); // stop unit testing
}

void loop()
{
    // Blink to indicate end of test
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
}