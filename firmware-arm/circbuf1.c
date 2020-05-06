/* circbuf1.c --- byte circular buffer */

/* $Id: $ */

#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>

#include "circbuf1.h"

/* Initializes fields of queue */
circbuf1_t * circbuf1_init(circbuf1_t * self, uint8_t * pbuf, uint size, void (*write_hook)(void)) {
  self->size = size;
  self->pRD = self->pWR = 0;
  self->pbuf = pbuf; 
  self->write_hook = write_hook;
  return self;
}

/*
  Post a byte to the circbuf.

  Return 0 if buffer is full before write data, and do not write that data.
  Return non zero if succeeded.
*/
int circbuf1_put(circbuf1_t * self, int data) {
  /* verify if buffer is full */
  if (self->pWR == self->size - 1) {
    if (self->pRD == 0)
      return 0;
  }
  else if (self->pWR == self->pRD -1)
    return 0;
  /* put data in circbuf */
  *(self->pbuf + self->pWR) = (uint8_t) data;
  /* inc pWR: */
  self->pWR++;
  if (self->pWR >= self->size)
    self->pWR = 0;
  /* success */
  return 1;
}

/*
  Read a byte from the circbuf
  Return less than zero if buffer is empty before read, and do not read nothing.
 */
int circbuf1_get(circbuf1_t * self) {
  int data;
  /* verify if it is empty */
  if (circbuf1_is_empty(self))
    return -1;
  /* read data */
  data = *(self->pbuf + self->pRD);
  /* inc pRD: */
  self->pRD++;
  if (self->pRD >= self->size)
    self->pRD = 0;
  return data;
}

/* verify if circbuf has no new data */
int circbuf1_is_empty(circbuf1_t * self) {
  int res;
  res = (self->pRD == self-> pWR);
  return res;
}

