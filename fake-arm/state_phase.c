#include <stdlib.h>
#include "state_phase.h"

/* global phase number 0 to n-1 */
static int phase = -1;

/* phase interval counter */
static int phase_i = 0;

/* flags a new phase for logging */
static int phase_new = 1;

/* records last phase */
static int phase_last = -1;

static int number_phases;

static phase_fcn * phase_table;

/*To be called each time phase is changed */
static void (*phase_log)(void) = NULL;

/* to be called after every time phase function is executed */
static void (*state_eqns)(void) = NULL;

int phase_get(void) {
  return phase;
}

void phase_exec(void) {
  if (phase_new) {
    /* calls phase log */
    if (phase_log)
      phase_log();
    phase_new = 0;
  }
  phase_table[phase](phase_i);
  /* do state equation functions */
  if (state_eqns)
    state_eqns();
}

void phase_set(int next_phase) {
  /* resets phase counter, so it knows when changing to a new phase */
  phase_i = 0;
  if (phase_last != phase)
    phase_new = 1;
  phase_last = phase;
  phase = next_phase;
  if (phase >= number_phases) phase = 0;
}

void phase_next(void) {
  /* resets phase counter, so it knows when changing to a new phase */
  phase_i = 0;
  phase_new = 1;
  /* save last phase number */
  phase_last = phase;
  phase++;
  if (phase >= number_phases) phase = 0;
}

void phase_i_inc(void) {
  phase_i++;
}

void phase_init(int initial_phase, void (*phase_log_fcn)(void), int number_of_phases, phase_fcn *ext_phase_table, void (*phase_state_eqns)(void)) {
  phase = initial_phase;
  phase_table = ext_phase_table;
  state_eqns = phase_state_eqns;
  number_phases = number_of_phases;
  phase_log = phase_log_fcn;
}
