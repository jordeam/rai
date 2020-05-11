#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "interpreter.h"
#include "parameters.h"
#include "commands.h"

/* Prefixes-> 'r': ventilator, 'i':infusion pump, 'm': cardiac monitor, 'o': dissolved oxigen */
const command_table_t cmdtab[] = { {"firmware", RO, pstring, offsetof(global_parameters_t, firmware), NULL, NULL, "Software firmware"},
                                   {"r:stop", EXEO, none, 0, ventilator_stop, NULL, "Stop ventilator"},
                                   {"r:start", EXEO, none, 0, ventilator_start, NULL, "Resume ventilator operation"},
                                   {"r:mode", RW, uint8, offsetof(ventilator_parameters_t, mode), NULL, NULL, "Ventilator mode"},
                                   {"r:air", RW, uint32, offsetof(ventilator_parameters_t, air), NULL, NULL, "Inspired air volume"},
                                   {"r:O2", RW, uint32, offsetof(ventilator_parameters_t, O2), NULL, NULL, "Inspired O2 volume"},
                                   {"r:insp-time", RW, uint32, offsetof(ventilator_parameters_t, insp_time), NULL, NULL, "Inspired O2 volume"},
                                   {"r:vexp", RW, uint32, offsetof(ventilator_parameters_t, vexp), NULL, NULL, "Forced exalating volume"},
                                   {"r:texpf", RW, uint32, offsetof(ventilator_parameters_t, texpf), NULL, NULL, "Forced exalating time"},
                                   {"r:texpn", RW, uint32, offsetof(ventilator_parameters_t, texpn), NULL, NULL, "Natural exalating time"},
                                   {"r:ppress", RW, uint32, offsetof(ventilator_parameters_t, ppress), NULL, NULL, "Positive pressure"},
                                   {"r:npress", RW, uint32, offsetof(ventilator_parameters_t, npress), NULL, NULL, "Negative pressure"},
                                   {"i:calib", EXEO, none, 0, infpump_calib, NULL, "Calibrate/reset pump"},
                                   {"i:start", EXEO, none, 0, infpump_start, NULL, "Start/resume pump"},
                                   {"i:stop", EXEO, none, 0, infpump_stop, NULL, "Stop pump"},
                                   {"i:steps", RO, int32, offsetof(infusionpump_parameters_t, steps), NULL, NULL, "Actual position"},
                                   {"i:max-steps", RO, int32, offsetof(infusionpump_parameters_t, max_steps), NULL, NULL, "Maximum steps"},
                                   {"i:ser", RO, uint8, offsetof(infusionpump_parameters_t, ser), NULL, NULL, "Syringe sensor"},
                                   {"i:go", RW, int32, offsetof(infusionpump_parameters_t, go), NULL, NULL, "Remaining steps"},
                                   {"i:time_step", RW, uint32, offsetof(infusionpump_parameters_t, time_step), NULL, NULL, "Time steps"},
                                   {NULL}};

/*
  Ventilator exec commands
*/
int ventilator_stop(char *cmd) {
  printf("Stoping venilator proccess, puts cylinder in its origin.\n");
  return 0;
}

/*
  Starting/Resuming ventilator proccess.
*/
int ventilator_start(char *cmd) {
  printf("Starting ventilator process.\n");
  return 0;
}

/*
  Starting/Resuming infusion pump process
*/
int infpump_start(char *cmd){
  int pump = cmd[0] - '0';
  printf("Starting pump %d.\n", pump);
  return 0;
}

/*
  Stoping infusion pump process
*/
int infpump_stop(char *cmd) {
  int pump = cmd[0] - '0';
  printf("Starting pump %d.\n", pump);
  return 0;
}

/*
  Calibrating infusion pump
*/
int infpump_calib(char *cmd) {
  int pump = cmd[0] - '0';
  printf("Calibrating pump %d.\n", pump);
  return 0;
}
