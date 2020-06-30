#include <stdlib.h>

#include "oper_parameters.h"

global_parameters_t global_params;
ventilator_parameters_t vent_params;
infusionpump_parameters_t infpump_params[4];

double end_time = -1;

extern float FIO2, VolINS, t_INS, VolEXPF, t_EXPF, t_EXPN;

const void * baseaddr_params[] = { &global_params, &vent_params, &infpump_params, NULL, NULL };

void oper_parameters_init(void) {
  int i;
  global_params.firmware = FIRMWARE_VERSION;
  
  vent_params.mode = 0;
  vent_params.air = VolINS * 1e6;
  vent_params.FIO2 = FIO2 * 100;
  vent_params.insp_time = t_INS * 1e3;
  vent_params.vexp = VolEXPF * 1e6;
  vent_params.texpf = t_EXPF * 1e3;
  vent_params.texpn = t_EXPN * 1e3;
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
