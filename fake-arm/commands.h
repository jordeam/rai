#ifndef _commands_h
#define _commands_h

#include "interpreter.h"

int ventilator_start(char *cmd);
int ventilator_stop(char *cmd);
int infpump_start(char *cmd);
int infpump_stop(char *cmd);
int infpump_calib(char *cmd);

extern const command_table_t cmdtab[];

#endif
