#ifndef _state_phase_h
#define _state_phase_h

/* it must returns new values of phase_i */
typedef int (*phase_fcn)(int phase_i);

void phase_init(int initial_phase, void (*on_phase_changed)(void), int number_of_phases, phase_fcn *ext_phase_table, void (*state_eqns)(void));
void phase_exec(void);
void phase_set(int next_phase);
void phase_next(void);
int phase_get(void);

#endif
