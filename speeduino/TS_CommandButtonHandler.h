
/** \file
 * Header file for the TunerStudio command handler
 * The command handler manages all the inputs FROM TS which are issued when a command button is clicked by the user
 */

#define TS_CMD_TEST_DSBL    256U
#define TS_CMD_TEST_ENBL    257U

#define TS_CMD_INJ1_ON      513U
#define TS_CMD_INJ1_OFF     514U
#define TS_CMD_INJ1_PULSED  515U
#define TS_CMD_INJ2_ON      516U
#define TS_CMD_INJ2_OFF     517U
#define TS_CMD_INJ2_PULSED  518U
#define TS_CMD_INJ3_ON      519U
#define TS_CMD_INJ3_OFF     520U
#define TS_CMD_INJ3_PULSED  521U
#define TS_CMD_INJ4_ON      522U
#define TS_CMD_INJ4_OFF     523U
#define TS_CMD_INJ4_PULSED  524U
#define TS_CMD_INJ5_ON      525U
#define TS_CMD_INJ5_OFF     526U
#define TS_CMD_INJ5_PULSED  527U
#define TS_CMD_INJ6_ON      528U
#define TS_CMD_INJ6_OFF     529U
#define TS_CMD_INJ6_PULSED  530U
#define TS_CMD_INJ7_ON      531U
#define TS_CMD_INJ7_OFF     532U
#define TS_CMD_INJ7_PULSED  533U
#define TS_CMD_INJ8_ON      534U
#define TS_CMD_INJ8_OFF     535U
#define TS_CMD_INJ8_PULSED  536U

#define TS_CMD_IGN1_ON      769U
#define TS_CMD_IGN1_OFF     770U
#define TS_CMD_IGN1_PULSED  771U
#define TS_CMD_IGN2_ON      772U
#define TS_CMD_IGN2_OFF     773U
#define TS_CMD_IGN2_PULSED  774U
#define TS_CMD_IGN3_ON      775U
#define TS_CMD_IGN3_OFF     776U
#define TS_CMD_IGN3_PULSED  777U
#define TS_CMD_IGN4_ON      778U
#define TS_CMD_IGN4_OFF     779U
#define TS_CMD_IGN4_PULSED  780U
#define TS_CMD_IGN5_ON      781U
#define TS_CMD_IGN5_OFF     782U
#define TS_CMD_IGN5_PULSED  783U
#define TS_CMD_IGN6_ON      784U
#define TS_CMD_IGN6_OFF     785U
#define TS_CMD_IGN6_PULSED  786U
#define TS_CMD_IGN7_ON      787U
#define TS_CMD_IGN7_OFF     788U
#define TS_CMD_IGN7_PULSED  789U
#define TS_CMD_IGN8_ON      790U
#define TS_CMD_IGN8_OFF     791U
#define TS_CMD_IGN8_PULSED  792U

#define TS_CMD_STM32_REBOOT     12800
#define TS_CMD_STM32_BOOTLOADER 12801

#define TS_CMD_SD_FORMAT  13057

#define TS_CMD_VSS_60KMH  39168 //0x99x00
#define TS_CMD_VSS_RATIO1 39169
#define TS_CMD_VSS_RATIO2 39170
#define TS_CMD_VSS_RATIO3 39171
#define TS_CMD_VSS_RATIO4 39172
#define TS_CMD_VSS_RATIO5 39173
#define TS_CMD_VSS_RATIO6 39174

/* the maximum id number is 65,535 */
bool TS_CommandButtonsHandler(uint16_t buttonCommand);
