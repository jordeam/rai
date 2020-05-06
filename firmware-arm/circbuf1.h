/* circbuf1.h --- byte circular buffer */

/* $Id: $ */

#ifndef _circbuf1_h
#define _circbuf1_h

#include <sys/types.h>
#include <stdlib.h>

/* Byte circbuf definition: */
struct circbuf1 {
  uint size;
  uint pWR, pRD;
  uint8_t * pbuf;
  void (*write_hook)(void);
};

typedef struct circbuf1 circbuf1_t;

circbuf1_t * circbuf1_init(circbuf1_t * self, uint8_t * pbuf, uint size, void (*write_hook)(void));
int circbuf1_put(circbuf1_t * self, int data);
int circbuf1_get(circbuf1_t * self);
int circbuf1_is_empty(circbuf1_t * self);

#endif
