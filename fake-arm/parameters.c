#include <stdlib.h>

#include "parameters.h"

global_parameters_t global_params;
ventilator_parameters_t vent_params;
infusionpump_parameters_t infpump_params[4];

const void * baseaddr_params[] = { &global_params, &vent_params, &infpump_params, NULL, NULL };

void init_parameters(void) {
  int i;
  global_params.firmware = FIRMWARE_VERSION;
  
  vent_params.mode = 0;
  vent_params.air = 200;
  vent_params.O2 = 0;
  vent_params.insp_time = 400;
  vent_params.vexp = 0;
  vent_params.texpf = 0;
  vent_params.texpn = 600;
  vent_params.ppress = 80;
  vent_params.npress = 20;

  for (i = 0; i < 4; i++) {
    infpump_params[i].steps = -1;
    infpump_params[i].max_steps = -1;
    infpump_params[i].ser = 0;
    infpump_params[i].go = 0;
    infpump_params[i].time_step = 200;
  }
}
