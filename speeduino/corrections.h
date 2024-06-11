/*
All functions in the gamma file return

*/
#ifndef CORRECTIONS_H
#define CORRECTIONS_H

void initialiseFuelCorrections(statuses &current);
uint16_t correctionsFuel(void);
uint8_t calculateAfrTarget(const table3d16RpmLoad &afrLookUpTable, const statuses &current, const config2 &page2, const config6 &page6);

void initialiseIgnCorrections(statuses &current);
int8_t correctionsIgn(int8_t advance);
int8_t correctionFixedTiming(int8_t advance, const config2 &page2, const config4 &page4);
int8_t correctionCrankingFixedTiming(int8_t advance, const statuses &current, const config2 &page2, const config4 &page4, const table2D &cltAdvanceLUT);

uint16_t correctionsDwell(uint16_t dwell, statuses &current, const config2 &page2, const config4 &page4, const config10 &page10, const table2D &lookupTable);


#endif // CORRECTIONS_H
