#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "interpreter.h"
#include "parameters.h"
#include "commands.h"

extern float VolINS;
int air_post(char *cmd, char *data) {
  VolINS = vent_params.air * 1e-6;
  return 0;
}

extern float FIO2;
int FIO2_post(char *cmd, char *data) {
  if (vent_params.FIO2 > 100) vent_params.FIO2 = 100;
  else if (vent_params.FIO2 < 20) vent_params.FIO2 = 20;
  FIO2 = vent_params.FIO2 * 1e-2;
  return 0;
}

extern float T_INS;
int insp_time_post(char *cmd, char *data) {
  T_INS = vent_params.insp_time * 1e-3;
  return 0;
}

extern float VolEXPF;
int vexp_post(char *cmd, char *data) {
  VolEXPF = vent_params.vexp * 1e-6;
  return 0;
}

extern float T_EXPF;
int texpf_post(char *cmd, char *data) {
  T_EXPF = vent_params.texpf * 1e-3;
  return 0;
}

extern float T_EXPN;
int texpn_post(char *cmd, char *data) {
  T_EXPN = vent_params.texpn * 1e-3;
  return 0;
}

/* Prefixes-> 'r': ventilator, 'i':infusion pump, 'm': cardiac monitor, 'o': dissolved oxigen */
const command_table_t cmdtab[] = { {"firmware", RO, pstring, offsetof(global_parameters_t, firmware), NULL, NULL, "Software firmware"},
                                   {"r:stop", EXEO, none, 0, ventilator_stop, NULL, "Stop ventilator"},
                                   {"r:start", EXEO, none, 0, ventilator_start, NULL, "Resume ventilator operation"},
                                   {"r:mode", RW, uint8, offsetof(ventilator_parameters_t, mode), NULL, NULL, "Ventilator mode"},
                                   {"r:air", RW, uint32, offsetof(ventilator_parameters_t, air), NULL, air_post, "Inspired air volume"},
                                   {"r:FIO2", RW, uint32, offsetof(ventilator_parameters_t, FIO2), NULL, FIO2_post, "Inspired % O2 volume"},
                                   {"r:insp-time", RW, uint32, offsetof(ventilator_parameters_t, insp_time), NULL, insp_time_post, "Inspiration time"},
                                   {"r:vexp", RW, uint32, offsetof(ventilator_parameters_t, vexp), NULL, vexp_post, "Forced exalating volume"},
                                   {"r:texpf", RW, uint32, offsetof(ventilator_parameters_t, texpf), NULL, texpf_post, "Forced exalating time"},
                                   {"r:texpn", RW, uint32, offsetof(ventilator_parameters_t, texpn), NULL, texpn_post, "Natural exalating time"},
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
extern int ventilator_run;
int ventilator_stop(char *cmd) {
  printf("Stoping venilator proccess, puts cylinder in its origin.\n");
  ventilator_run = 0;
  return 0;
}

/*
  Starting/Resuming ventilator proccess.
*/
int ventilator_start(char *cmd) {
  printf("Starting ventilator process.\n");
  ventilator_run = 1;
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
