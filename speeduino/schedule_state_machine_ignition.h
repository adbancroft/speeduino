#pragma once

#include "scheduler.h"

/**
 * @brief Shared ignition schedule timer ISR *implementation*. Should be called by the actual ignition timer ISRs
 * (as timed interrupts) when either the start time or the duration time are reached. See @ref schedule-state-machine
 * 
 * @param schedule The ignition schedule to move to the next state
 */
void moveToNextState(IgnitionSchedule &schedule);