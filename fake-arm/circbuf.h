#ifndef _data_log_h
#define _data_log_h

struct circbufrw {
  /* index for write, write and increment */
  int wi;
  /* back position: index for read, read and increment */
  int ri;
  /* maximum number of elements */
  int n;
};

typedef struct circbufrw circbufrw_t;

int circbufrw_inc_ri(circbufrw_t * self);
int circbufrw_inc_ri_n(circbufrw_t * self, int n);
int circbufrw_inc_wi(circbufrw_t * self);
int circbufrw_get_size(circbufrw_t * self);
void circbufrw_init(circbufrw_t * self, int NumLogs);

#endif
