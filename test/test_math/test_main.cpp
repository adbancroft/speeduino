#include <unity.h>
#include "test_saturated_cast.h"
#include "test_muldiv.h"
#include "test_rescale.h"
#include "tests_crankmaths.h"


extern void testPercent(void);
extern void testDivision(void);


void run_tests(void)
{
    UNITY_BEGIN();    // IMPORTANT LINE!

    test_saturated_cast();
    testmuldiv();
    testrescale();
    testCrankMaths();
    testPercent();
    testDivision();

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

// Native build
int main(int argc, char **argv) {
  run_tests();
  return 0;
}
#endif