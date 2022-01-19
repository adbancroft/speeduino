/** @file
 * Instantiation of various (table2D, table3D) tables, volatile (interrupt modified) variables, Injector (1...8) enablement flags, etc.
 */
#include "globals.h"
#include "utilities.h"
#include "scheduler.h"
#include "scheduledIO_direct.h"

const char TSfirmwareVersion[] PROGMEM = "Speeduino";

const byte data_structure_version = 2; //This identifies the data structure when reading / writing. (outdated ?)

struct table3d16RpmLoad fuelTable; ///< 16x16 fuel map
struct table3d16RpmLoad fuelTable2; ///< 16x16 fuel map
struct table3d16RpmLoad ignitionTable; ///< 16x16 ignition map
struct table3d16RpmLoad ignitionTable2; ///< 16x16 ignition map
struct table3d16RpmLoad afrTable; ///< 16x16 afr target map
struct table3d8RpmLoad stagingTable; ///< 8x8 fuel staging table
struct table3d8RpmLoad boostTable; ///< 8x8 boost map
struct table3d8RpmLoad boostTableLookupDuty; ///< 8x8 boost map lookup table
struct table3d8RpmLoad vvtTable; ///< 8x8 vvt map
struct table3d8RpmLoad vvt2Table; ///< 8x8 vvt2 map
struct table3d8RpmLoad wmiTable; ///< 8x8 wmi map
struct table3d4RpmLoad dwellTable; ///< 4x4 Dwell map

struct table2D<uint8_t, uint8_t, 4> taeTable(configPage4.taeBins, configPage4.taeValues); ///< 4 bin TPS Acceleration Enrichment map (2D)
struct table2D<uint8_t, uint8_t, 4> maeTable(configPage4.maeBins, configPage4.maeRates);
struct table2D<uint8_t, uint8_t, 10> WUETable(configPage4.wueBins, configPage2.wueValues); ///< 10 bin Warm Up Enrichment map (2D)
struct table2D<uint8_t, uint8_t, 4> ASETable(configPage2.aseBins, configPage2.asePct); ///< 4 bin After Start Enrichment map (2D)
struct table2D<uint8_t, uint8_t, 4> ASECountTable(configPage2.aseBins, configPage2.aseCount); ///< 4 bin After Start duration map (2D)
struct table2D<uint8_t, uint8_t, 4> PrimingPulseTable(configPage2.primeBins, configPage2.primePulse); ///< 4 bin Priming pulsewidth map (2D)
struct table2D<uint8_t, uint8_t, 4> crankingEnrichTable(configPage10.crankingEnrichBins, configPage10.crankingEnrichValues); ///< 4 bin cranking Enrichment map (2D)
struct table2D<uint8_t, uint8_t, 6> dwellVCorrectionTable(configPage6.voltageCorrectionBins, configPage4.dwellCorrectionValues); ///< 6 bin dwell voltage correction (2D)
struct table2D<uint8_t, uint8_t, 6> injectorVCorrectionTable(configPage6.voltageCorrectionBins, configPage6.injVoltageCorrectionValues); ///< 6 bin injector voltage correction (2D)
struct table2D<uint8_t, uint16_t, 4> injectorAngleTable(configPage2.injAngRPM, configPage2.injAng); ///< 4 bin injector angle curve (2D)
struct table2D<uint8_t, uint8_t, 9> IATDensityCorrectionTable(configPage6.airDenBins, configPage6.airDenRates); ///< 9 bin inlet air temperature density correction (2D)
struct table2D<uint8_t, uint8_t, 8> baroFuelTable(configPage4.baroFuelBins, configPage4.baroFuelValues); ///< 8 bin baro correction curve (2D)
struct table2D<uint8_t, uint8_t, 6> IATRetardTable(configPage4.iatRetBins, configPage4.iatRetValues); ///< 6 bin ignition adjustment based on inlet air temperature  (2D)
struct table2D<uint8_t, uint8_t, 10> idleTargetTable(configPage6.iacBins, configPage6.iacCLValues); ///< 10 bin idle target table for idle timing (2D)
struct table2D<uint8_t, uint8_t, 6> idleAdvanceTable(configPage4.idleAdvBins, configPage4.idleAdvValues); ///< 6 bin idle advance adjustment table based on RPM difference  (2D)
struct table2D<uint8_t, uint8_t, 6> CLTAdvanceTable(configPage4.cltAdvBins, configPage4.cltAdvValues); ///< 6 bin ignition adjustment based on coolant temperature  (2D)
struct table2D<uint8_t, uint8_t, 8> rotarySplitTable(configPage10.rotarySplitBins, configPage10.rotarySplitValues); ///< 8 bin ignition split curve for rotary leading/trailing  (2D)
struct table2D<uint8_t, uint8_t, 6> flexFuelTable(configPage10.flexFuelBins, configPage10.flexFuelAdj);  ///< 6 bin flex fuel correction table for fuel adjustments (2D)
struct table2D<uint8_t, uint8_t, 6> flexAdvTable(configPage10.flexAdvBins, configPage10.flexAdvAdj);   ///< 6 bin flex fuel correction table for timing advance (2D)
struct table2D<uint8_t, int16_t, 6> flexBoostTable(configPage10.flexBoostBins, configPage10.flexBoostAdj); ///< 6 bin flex fuel correction table for boost adjustments (2D)
struct table2D<uint8_t, uint8_t, 6> fuelTempTable(configPage10.fuelTempBins, configPage10.fuelTempValues);  ///< 6 bin flex fuel correction table for fuel adjustments (2D)
struct table2D<uint8_t, uint8_t, 6> knockWindowStartTable(configPage10.knock_window_rpms, configPage10.knock_window_angle);
struct table2D<uint8_t, uint8_t, 6> knockWindowDurationTable(configPage10.knock_window_rpms, configPage10.knock_window_dur);
struct table2D<uint8_t, uint8_t, 4> oilPressureProtectTable(configPage10.oilPressureProtRPM, configPage10.oilPressureProtMins);
struct table2D<uint8_t, uint8_t, 6> wmiAdvTable(configPage10.wmiAdvBins, configPage10.wmiAdvAdj); //6 bin wmi correction table for timing advance (2D)
struct table2D<uint8_t, uint8_t, 6> coolantProtectTable(configPage9.coolantProtRPM, configPage9.coolantProtTemp); //6 bin coolant temperature protection table for engine protection (2D)
struct table2D<uint8_t, uint8_t, 4> fanPWMTable(configPage6.fanPWMBins, configPage9.PWMFanDuty);
struct table2D<int8_t, uint8_t, 4> rollingCutTable(configPage15.rollingProtRPMDelta, configPage15.rollingProtCutPercent);


//These are variables used across multiple files
byte fpPrimeTime = 0; ///< The time (in seconds, based on @ref statuses.secl) that the fuel pump started priming
uint8_t softLimitTime = 0; //The time (in 0.1 seconds, based on seclx10) that the soft limiter started
volatile uint16_t mainLoopCount; //Main loop counter (incremented at each main loop rev., used for maintaining currentStatus.loopsPerSecond)
volatile uint32_t toothHistory[TOOTH_LOG_SIZE]; ///< Tooth trigger history - delta time (in uS) from last tooth (Indexed by @ref toothHistoryIndex)
volatile uint8_t compositeLogHistory[TOOTH_LOG_SIZE]; 
volatile uint8_t toothHistoryIndex = 0; ///< Current index to @ref toothHistory array
unsigned long currentLoopTime; /**< The time (in uS) that the current mainloop started */
volatile uint16_t ignitionCount; /**< The count of ignition events that have taken place since the engine started */
int16_t CRANK_ANGLE_MAX_IGN = 360;
int16_t CRANK_ANGLE_MAX_INJ = 360; ///< The number of crank degrees that the system track over. 360 for wasted / timed batch and 720 for sequential
volatile uint32_t runSecsX10;
volatile uint32_t seclx10;
volatile byte HWTest_INJ = 0; /**< Each bit in this variable represents one of the injector channels and it's HW test status */
volatile byte HWTest_INJ_Pulsed = 0; /**< Each bit in this variable represents one of the injector channels and it's pulsed HW test status */
volatile byte HWTest_IGN = 0; /**< Each bit in this variable represents one of the ignition channels and it's HW test status */
volatile byte HWTest_IGN_Pulsed = 0; 

//This needs to be here because using the config page directly can prevent burning the setting
byte resetControl = RESET_CONTROL_DISABLED;

volatile byte TIMER_mask;
volatile byte LOOP_TIMER;

struct statuses currentStatus; /**< The master global "live" status struct. Contains all values that are updated frequently and used across modules */
struct config2 configPage2;
struct config4 configPage4;
struct config6 configPage6;
struct config9 configPage9;
struct config10 configPage10;
struct config13 configPage13;
struct config15 configPage15;

uint16_t cltCalibration_bins[32];
uint16_t cltCalibration_values[32];
struct table2D<uint16_t, uint16_t, 32> cltCalibrationTable(cltCalibration_bins, cltCalibration_values);
uint16_t iatCalibration_bins[32];
uint16_t iatCalibration_values[32];
struct table2D<uint16_t, uint16_t, 32> iatCalibrationTable(iatCalibration_bins, iatCalibration_values);
uint16_t o2Calibration_bins[32];
uint8_t o2Calibration_values[32];
struct table2D<uint16_t, uint8_t, 32> o2CalibrationTable(o2Calibration_bins, o2Calibration_values); 
