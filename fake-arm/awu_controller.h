#ifndef _awu_controller_h
#define _awu_controller_h

#include <stdint.h>

struct awu_controller {
  float uk0, ak0, kp, ki, max, min;
};

typedef struct awu_controller awu_controller_t;

float awu_controller_eval(awu_controller_t * self, float ref, float val);
void awu_controller_init(awu_controller_t *self, float kp, float ki, float max, float min);

#endif
