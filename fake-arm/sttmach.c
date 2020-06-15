#include <stdlib.h>
#include "sttmach.h"

/* global phase number 0 to n-1 */
static int cur_state;

/* state interval counter, autmocally incremented each time state is called */
static int phase_i = 0;

/* records last state */
static int last_state = -1;

static int number_of_states;

static sttmach_fcn * phase_table;

/* to be used outside a state function, if necessary */
int sttmach_get_cur_state(void) {
  return cur_state;
}

/* to be used outside a state function, if necessary */
int sttmach_get_phase_i(void) {
  return phase_i;
}

void sttmach_exec(void) {
  int next_state;
  next_state = phase_table[cur_state](cur_state, phase_i);
  /* automatically increments counter */
  phase_i++;
  /* sanitize next_state */
  if (next_state >= number_of_states) next_state = 0;
  /* verify if state is to be changed */
  if (next_state != cur_state) {
    phase_i = 0;
    last_state = cur_state;
    cur_state = next_state;
  }
}

void sttmach_init(int initial_state, int _number_of_states, sttmach_fcn *_phase_table) {
  cur_state = initial_state;
  phase_table = _phase_table;
  number_of_states = _number_of_states;
}
