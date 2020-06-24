#include "mymath.h"
#include "awu_controller.h"

/* Voltage controller, current reference as output  */
float awu_controller_eval(awu_controller_t * self, float ref, float val) {
  float ak;
  float uk;
  float err = ref - val;
  ak = self->kp * err;
  uk = ak - self->ak0 + self->ki * err + self->uk0;
  uk = saturate(uk, self->max, self->min);
  self->ak0 = saturate(ak, self->max, self->min);
  self->uk0 = uk;
  return uk;
}

void awu_controller_init(awu_controller_t *self, float kp, float ki, float max, float min) {
  self->ak0 = self->uk0 = 0;
  self->kp = kp;
  self->ki = ki;
  self->min = min;
  self->max = max;
}
