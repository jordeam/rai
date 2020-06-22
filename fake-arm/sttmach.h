#ifndef _sttmach_h
#define _sttmach_h

/* it must returns new state function pointer value or current state function pointer if it is unchanged. */

/* it must returns new state function pointer value or current state function pointer if it is unchanged. */
typedef void * (*sttmach_fcn)(struct sttmach * self);

struct sttmach {
  /* id of current state, to log purposes */
  int id;
  /* current state function pointer */
  sttmach_fcn state;
  /* state interval counter, autmatically incremented each time state is called */
  int i;
};

typedef struct sttmach sttmach_t;

void sttmach_init(sttmach_t * self, sttmach_fcn initial_phase);
void sttmach_exec(sttmach_t * self);

#endif
