#include <stdlib.h>
#include "sttmach.h"

void sttmach_exec(sttmach_t * self) {
  sttmach_fcn next_state;
  next_state = self->state(self);
  /* automatically increments counter */
  self->i++;
  /* verify if state is to be changed */
  if (next_state != self->state) {
    self->i = 0;
    self->state = next_state;
  }
}

void sttmach_init(sttmach_t * self, sttmach_fcn initial_state) {
  self->state = initial_state;
  self->i = 0;
}
