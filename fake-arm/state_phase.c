#include <stdlib.h>
#include "state_phase.h"

/* phase internal counter */
int phase_i = 0;

/* global phase number 0 to n-1 */
int phase = -1;

static int number_phases;

static phase_table_entry_t * phase_table;

/*To be called each time phase is changed */
static void (*phase_log)(void) = NULL;

/* to be called after every time phase function is executed */
static void (*state_eqns)(void) = NULL;

void do_phase(int phase) {
  static int last_phase = -1, phase_init_ok = 0;
  if (last_phase != phase) {
    /* exec last phase post */
    if (last_phase > 0 && phase_table[last_phase].post)
      phase_table[last_phase].post();
    /* calls phase log */
    if (phase_log)
      phase_log();
    /* calls pre */
    if (phase_table[phase].pre)
      phase_init_ok = phase_table[phase].pre();
    else
      /* set init ok if there is no pre-phase */
      phase_init_ok = 1;
    if (phase_init_ok && phase_log)
      /* calls phase_log routine if it is successfull */
      phase_log();
    last_phase = phase;
  }
  /* try again, could be failed due to a data lock, for ex. */
  if (!phase_init_ok) {
    phase_init_ok = phase_table[phase].pre();
    if (phase_init_ok && phase_log)
      /* calls phase_log routine if it is successfull */
      phase_log();
  }
  /* do phase only if init succeed */
  if (phase_init_ok && phase_table[phase].exec)
    phase_table[phase].exec();
  /* do state equation functions */
  if (state_eqns)
    state_eqns();
  /* increments phase internal counter */
  phase_i++;
}

void switch_to_phase(int next_phase) {
  phase = next_phase;
  if (phase >= number_phases) phase = 0;
}

void switch_to_next_phase(void) {
  phase++;
  if (phase >= number_phases) phase = 0;
}

void phase_init(void (*phase_log_fcn)(void), int number_of_phases, phase_table_entry_t *ext_phase_table, void (*phase_state_eqns)(void)) {
  phase = 0;
  phase_table = ext_phase_table;
  state_eqns = phase_state_eqns;
  number_phases = number_of_phases;
  phase_log = phase_log_fcn;
}
