#include <stdlib.h>
#include "sttmach.h"

/* to be used outside a state function, if necessary, e.g., or log purposes */
int sttmach_get_cur_state(sttmach_t * self) {
  return (int) self->state(-1);
}

void sttmach_exec(sttmach_t * self) {
  sttmach_fcn next_state;
  next_state = self->state(self->phase_i);
  /* automatically increments counter */
  self->phase_i++;
  /* verify if state is to be changed */
  if (next_state != self->state) {
    self->phase_i = 0;
    self->state = next_state;
  }
}

void sttmach_init(sttmach_t * self, sttmach_fcn initial_state) {
  self->state = initial_state;
}
