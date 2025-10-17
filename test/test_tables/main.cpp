#include <Arduino.h>
#include <unity.h>
#include <avr/sleep.h>

extern void testTables3d(void);
extern void testTable2d(void);
extern void testAlgorithms3D(void);

#define UNITY_EXCLUDE_DETAILS

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
#if !defined(SIMULATOR)
    delay(2000);
#endif

    UNITY_BEGIN();    // IMPORTANT LINE!

    testTables3d();
    testTable2d();
    testAlgorithms3D();

    UNITY_END(); // stop unit testing

#if defined(SIMULATOR)       // Tell SimAVR we are done
    cli();
    sleep_enable();
    sleep_cpu();
#endif  
}

void loop()
{
    // Blink to indicate end of test
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
}