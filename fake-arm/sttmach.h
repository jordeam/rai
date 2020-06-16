#ifndef _sttmach_h
#define _sttmach_h

/* it must returns new state function pointer value or current state function pointer if it is unchanged. */
typedef void * (*sttmach_fcn)(int phase_i);

struct sttmach {
  /* current state function pointer */
  sttmach_fcn state;
  /* state interval counter, autmatically incremented each time state is called */
  int phase_i;
};

typedef struct sttmach sttmach_t;

void sttmach_init(sttmach_t * self, sttmach_fcn initial_phase);
void sttmach_exec(sttmach_t * self);
int sttmach_get_cur_state(sttmach_t * self);

#endif
