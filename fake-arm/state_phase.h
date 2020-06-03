#ifndef _state_phase_h
#define _state_phase_h

/* phase internal counter */
extern int phase_i;

/* global phase number 0 to n-1 */
extern int phase;

struct phase_table_entry {
  /* returns 1 on success, 0 on fail: try again */
  int (*pre)(void);
  void (*exec)(void);
  void (*post)(void);
};

typedef struct phase_table_entry phase_table_entry_t;

void phase_init(void (*on_phase_changed)(void), int number_of_phases, phase_table_entry_t *ext_phase_table, void (*state_eqns)(void));
void do_phase(int phase);
void switch_to_phase(int next_phase);
void switch_to_next_phase(void);

#endif
