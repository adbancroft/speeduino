#ifndef SENSORS_H
#define SENSORS_H

#include "globals.h"

// The following are alpha values for the ADC filters.
// Their values are from 0 to 240, with 0 being no filtering and 240 being maximum
#define ADCFILTER_TPS_DEFAULT   50
#define ADCFILTER_CLT_DEFAULT  180
#define ADCFILTER_IAT_DEFAULT  180
#define ADCFILTER_O2_DEFAULT   128
#define ADCFILTER_BAT_DEFAULT  128
#define ADCFILTER_MAP_DEFAULT   20 //This is only used on Instantaneous MAP readings and is intentionally very weak to allow for faster response
#define ADCFILTER_BARO_DEFAULT  64

#define ADCFILTER_PSI_DEFAULT  150 //not currently configurable at runtime, used for misc pressure sensors, oil, fuel, etc.

#define FILTER_FLEX_DEFAULT     75

#define BARO_MIN      65
#define BARO_MAX      108

#define VSS_GEAR_HYSTERESIS 10
#define VSS_SAMPLES         4 //Must be a power of 2 and smaller than 255

extern volatile byte flexCounter;
extern volatile unsigned long flexStartTime;
extern volatile unsigned long flexPulseWidth;

#if defined(CORE_AVR)
  #define READ_FLEX() ((*flex_pin_port & flex_pin_mask) ? true : false)
#else
  #define READ_FLEX() digitalRead(pinFlex)
#endif

#define ADMUX_DEFAULT_CONFIG  0x40 //AVCC reference, ADC0 input, right adjusted, ADC enabled

extern unsigned int MAPcount; //Number of samples taken in the current MAP cycle
extern uint32_t MAPcurRev; //Tracks which revolution we're sampling on
extern bool auxIsEnabled;
extern uint16_t MAPlast; /**< The previous MAP reading */
extern unsigned long MAP_time; //The time the MAP sample was taken
extern unsigned long MAPlast_time; //The time the previous MAP sample was taken

/**
 * @brief Simple low pass IIR filter macro for the analog inputs
 * This is effectively implementing the smooth filter from playground.arduino.cc/Main/Smooth
 * But removes the use of floats and uses 8 bits of fixed precision.
 */
#define ADC_FILTER(input, alpha, prior) (((long)input * (256 - alpha) + ((long)prior * alpha))) >> 8

void initialiseADC(void);
void flexPulse(void);
void knockPulse(void);
uint32_t vssGetPulseGap(byte toothHistoryIndex);
void vssPulse(void);
uint16_t getSpeed(void);
byte getGear(void);
byte getFuelPressure(void);
byte getOilPressure(void);
uint16_t readAuxanalog(uint8_t analogPin);
uint16_t readAuxdigital(uint8_t digitalPin);

#define TPS_READ_FREQUENCY  30 //ONLY VALID VALUES ARE 15 or 30!!!

/** @brief Define the TPS sensor read frequency. */
#if TPS_READ_FREQUENCY==30
#define TPS_READ_TIMER_BIT BIT_TIMER_30HZ
#elif TPS_READ_FREQUENCY==15
#define TPS_READ_TIMER_BIT BIT_TIMER_15HZ
#else
#error
#endif

/**
 * @brief Read the TPS sensor
 * 
 * We expect this to be called at TPS_READ_TIMER_BIT intervals
 */
void readTPS(bool useFilter=true); //Allows the option to override the use of the filter

/** @brief Define the coolant sensor read frequency. */
#define CLT_READ_TIMER_BIT BIT_TIMER_4HZ

/**
 * @brief Read the coolant sensor
 * 
 * We expect this to be called at CLT_READ_TIMER_BIT intervals
 */
void readCLT(bool useFilter=true); //Allows the option to override the use of the filter

/** @brief Define the IAT sensor read frequency. */
#define IAT_READ_TIMER_BIT BIT_TIMER_4HZ

/**
 * @brief Read the IAT sensor
 * 
 * We expect this to be called at IAT_READ_TIMER_BIT intervals
 */
void readIAT(void);

/** @brief Define the O2 sensor read frequency. */
#define O2_READ_TIMER_BIT BIT_TIMER_30HZ

/**
 * @brief Read the O2 sensor
 * 
 * We expect this to be called at O2_READ_TIMER_BIT intervals
 */
void readO2(void);
/** @copydoc readO2 */
void readO2_2(void);

/** @brief Define the battery sensor read frequency. */
#define BAT_READ_TIMER_BIT BIT_TIMER_4HZ

/**
 * @brief Read the battery sensor
 * 
 * We expect this to be called at BAT_READ_TIMER_BIT intervals
 */
void readBat(void);

/** @brief Define the baro sensor read frequency. */
#define BARO_READ_TIMER_BIT BIT_TIMER_1HZ

/**
 * @brief Read the baro sensor
 * 
 * We expect this to be called at BARO_READ_TIMER_BIT intervals
 */
void readBaro(void);

/** @brief Define the MAP sensor read frequency. */
#define MAP_READ_TIMER_BIT BIT_TIMER_1KHZ

/**
 * @brief Read the MAP sensor
 * 
 * We expect this to be called at MAP_READ_TIMER_BIT intervals
 */
void readMAP(void);

void instanteneousMAPReading(void);
uint8_t getAnalogKnock(void);

#endif // SENSORS_H
