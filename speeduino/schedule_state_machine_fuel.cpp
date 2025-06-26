#include "schedule_state_machine_fuel.h"
#include "schedule_state_machine.h"

void moveToNextState(FuelSchedule &schedule)
{
  movetoNextState(schedule, defaultPendingToRunning, defaultRunningToOff, defaultRunningToPending);
} 
