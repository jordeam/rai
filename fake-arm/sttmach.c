#include <stdlib.h>
#include "sttmach.h"

void sttmach_exec(sttmach_t * self) {
  if (self->delay)
    self->delay--;
  else {
    sttmach_fcn last_state = self->state;
    if (self->state) {
      self->state(self);
      if (last_state != self->state)
        self->i = 0;
      else
        /* automatically increments counter */
        self->i++;
    }
  }
}

void sttmach_init(sttmach_t * self, sttmach_fcn initial_state) {
  self->state = initial_state;
  self->i = 0;
  self->delay = 0;
}
