#include <stdlib.h>

#include "circbuf.h"

/* index for data_log */
static int lpos;

/* back position for data log */
static int blpos;
int numlogs;

/* 
   It increments write index and returns buffer's size
*/
int circbuf_write_inc(void) {
  lpos++;
  if (lpos >= numlogs) 
    lpos = 0;
  if (blpos < 0)
    blpos = 0;
  /* if buffer is full, incs read index */
  if (lpos == blpos) {
    blpos++;
    if (blpos >= numlogs)
      blpos = 0;
    /* return lost a data */
    return numlogs;
  }
  else
    /* no data loss */
    return 0;
}

int circbuf_get_write_index(void) {
  return lpos;
}

int circbuf_get_read_index() {
  return blpos;
}

/* 
   Increment buffer read index and returns buffer's size.
*/
int circbuf_read_inc(void) {
  if (blpos == lpos)
    /* log is empty in the beginning */
    return 0;
  else {
    blpos++;
    if (blpos >= numlogs)
      blpos = 0;
    if (blpos > lpos)
      return lpos + numlogs - blpos;
    else
      return lpos - blpos;
  }
}

/* 
   Increment buffer read index by n and returns buffer's size.
*/
int circbuf_read_inc_n(int n) {
  if (n < circbuf_get_buf_size()) {
    blpos += n;
    if (blpos >= numlogs)
      blpos -= numlogs;
  }
  else
    blpos = lpos;
  return circbuf_get_buf_size();
}

int circbuf_get_buf_size(void) {
  if (blpos > lpos)
    return lpos + numlogs - blpos;
  else
    return lpos - blpos;
}

void circbuf_init(int NumLogs) {
  numlogs = NumLogs;
  lpos = 0;
  blpos = 0;
}
