#ifndef _parameters_h
#define _parameters_h

#include <stdint.h>

#define FIRMWARE_VERSION "rai-1-20200508"

struct global_parameters {
  char * firmware;
};

typedef struct global_parameters global_parameters_t;

struct ventilator_parameters {
  uint8_t mode;
  uint32_t air;
  uint32_t FIO2;
  uint32_t insp_time;
  uint32_t texpn;
  uint32_t ppress;
  uint32_t npress;
};

typedef struct ventilator_parameters ventilator_parameters_t;

struct infusionpump_parameters {
  int32_t steps, max_steps, go;
  uint32_t time_step;
  uint8_t ser;
};

typedef struct infusionpump_parameters infusionpump_parameters_t;

extern global_parameters_t global_params;
extern ventilator_parameters_t vent_params;
extern infusionpump_parameters_t infpump_params[4];

extern double end_time;

extern const void * baseaddr_params[];

void oper_parameters_init(void);

#endif
