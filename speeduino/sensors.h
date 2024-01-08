#ifndef SENSORS_H
#define SENSORS_H

#include "globals.h"
#include "port_pin.h"
#include "pin_mapping.h"

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

#define BARO_MIN      65U
#define BARO_MAX      108U

#define KNOCK_MODE_OFF      0U
#define KNOCK_MODE_DIGITAL  1U
#define KNOCK_MODE_ANALOG   2U

#define VSS_GEAR_HYSTERESIS 10U
#define VSS_SAMPLES         4U //Must be a power of 2 and smaller than 255

#define TPS_READ_FREQUENCY  30U //ONLY VALID VALUES ARE 15 or 30!!!

extern volatile byte flexCounter;
extern volatile unsigned long flexStartTime;
extern volatile unsigned long flexPulseWidth;

#define READ_FLEX() readPin(pinMapping.inputs.pinFlex)

#define ADMUX_DEFAULT_CONFIG  0x40 //AVCC reference, ADC0 input, right adjusted, ADC enabled

extern volatile byte knockCounter;

extern unsigned int MAPcount; //Number of samples taken in the current MAP cycle
extern uint32_t MAPcurRev; //Tracks which revolution we're sampling on
extern bool auxIsEnabled;
extern uint16_t MAPlast; /**< The previous MAP reading */
extern unsigned long MAP_time; //The time the MAP sample was taken
extern unsigned long MAPlast_time; //The time the previous MAP sample was taken

void initialiseADC(const pin_mapping_t &pins);

#define VSS_MODE_OFF        0U
#define VSS_MODE_CANBUS     1U
#define VSS_MODE_PULSES_KM  2U // Interrupt driven pulses per kilometer
#define VSS_MODE_PULSES_MI  3U // Interrupt driven pulses per mile

void initialiseVss(const pin_mapping_t &pins);
static inline bool isVssEnabled(void) {
  return configPage2.vssMode != VSS_MODE_OFF;
}
static inline bool isVssModeInterrupt(void) {
  return configPage2.vssMode > VSS_MODE_CANBUS;
}


void initialiseTPS(const pin_mapping_t &pins);
void readTPS(bool useFilter=true); //Allows the option to override the use of the filter

// CTPS == closed throttle position sensor
static inline bool isCTPSEnabled(void) {
  return configPage2.CTPSEnabled != 0U;;
}

void initialiseCoreSensors(const pin_mapping_t &pins);  // Sensors that are always required
void initialiseNonCoreSensors(const pin_mapping_t &pins); // Sensors that are optional

void initialiseFlexFuel(const pin_mapping_t &pins);
void flexPulse(void);
static inline bool isFlexEnabled(void) {
  return configPage2.flexEnabled != 0U;
}

uint32_t vssGetPulseGap(byte toothHistoryIndex);
void vssPulse(void);
uint16_t getSpeed(void);
byte getGear(void);

byte getFuelPressure(void);
static inline bool isFuelPressureEnabled(void) {
  return configPage10.fuelPressureEnable!=0U;
}

byte getOilPressure(void);
static inline bool isOilPressureEnabled(void) {
  return configPage10.oilPressureEnable;
}

uint16_t readAuxanalog(uint8_t analogPin);
uint16_t readAuxdigital(uint8_t digitalPin);

void readCLT(bool useFilter=true); //Allows the option to override the use of the filter

void readIAT(void);

void readO2(void);
void readO2_2(void);

void readBat(void);

void initialiseMapBaroSensors(const pin_mapping_t &pins);
void readBaro(void);
static inline bool isExtBaroEnabled(void) {
  return configPage6.useExtBaro != 0U;
}
void readMAP(void);
void instanteneousMAPReading(void);
static inline bool isExhMAPEnabled(void) {
  return configPage6.useEMAP != 0U;
}

#endif // SENSORS_H
