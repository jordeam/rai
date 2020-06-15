#ifndef _sttmach_h
#define _sttmach_h

/* it must returns new state value or current state if it is unchanged. */
typedef int (*sttmach_fcn)(int cur_state, int phase_i);

void sttmach_init(int initial_phase, int number_of_states, sttmach_fcn *ext_phase_table);
void sttmach_exec(void);
int sttmach_get_cur_state(void);
int sttmach_get_phase_i(void);

#endif
