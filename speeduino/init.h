#ifndef INIT_H
#define INIT_H
#include "pin_mapping.h"

#if defined(UNIT_TEST)
pin_mapping_t initialiseAll(void);
#else
void initialiseAll(void);
#endif

#endif