#include <stdlib.h>

#include "circbuf.h"

/* 
   It increments write index and returns buffer's size
*/
int circbufrw_inc_wi(circbufrw_t * self) {
  self->wi++;
  if (self->wi >= self->n) 
    self->wi = 0;
  /* if buffer is full, incs read index */
  if (self->wi == self->ri) {
    self->ri++;
    if (self->ri >= self->n)
      self->ri = 0;
    return self->n;
  }
  else
    /* no data loss */
    return circbufrw_get_size(self);
}

/* 
   Increment buffer read index and returns buffer's size.
*/
int circbufrw_inc_ri(circbufrw_t * self) {
  if (self->ri == self->wi)
    /* log is empty in the beginning */
    return 0;
  else {
    self->ri++;
    if (self->ri >= self->n)
      self->ri = 0;
    return circbufrw_get_size(self);
  }
}

/* 
   Increment buffer read index by n and returns buffer's size.
*/
int circbufrw_inc_ri_n(circbufrw_t * self, int n) {
  if (n < self->n) {
    self->ri += n;
    if (self->ri >= self->n)
      self->ri -= self->n;
  }
  else
    self->ri = self->wi;
  return circbufrw_get_size(self);
}

/* return the number of elements stored in buffer */
int circbufrw_get_size(circbufrw_t * self) {
  if (self->ri > self->wi)
    return self->wi + self->n - self->ri;
  else
    return self->wi - self->ri;
}

void circbufrw_init(circbufrw_t * self, int NumLogs) {
  self->n = NumLogs;
  self->wi = 0;
  self->ri = 0;
}
