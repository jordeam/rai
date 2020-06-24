#ifndef _sttmach_h
#define _sttmach_h

#include <stdint.h>

/* it must returns new state function pointer value or current state function pointer if it is unchanged. */


struct sttmach {
  /* id of current state, to log purposes */
  int id;
  /* current state function pointer */
  void (*state)(struct sttmach*);
  /* state interval counter, autmatically incremented each time state is called */
  int i;
  /* a delay to call current or next state */
  uint32_t delay;
};

typedef struct sttmach sttmach_t;

/* it must returns new state function pointer value or current state function pointer if it is unchanged. */
typedef void (*sttmach_fcn)(struct sttmach * self);

void sttmach_init(sttmach_t * self, sttmach_fcn initial_phase);
void sttmach_exec(sttmach_t * self);

#endif
