#ifndef _stasks_h
#define _stasks_h

#include <stdint.h>

#define STASKSNUM 30

/* The stask control block */
struct stasks_cb {
  uint16_t count, status;
  int data;
  void (*f)(void);
};

enum {
  UNCHANGED = 0,
  /* it is just there, doing nothing */
  BLOCKED,
  /* it will be KILLED next time, cleaned from TCBs, before being run */
  KILLED,
  /* will be executed */
  READY,
  /* will be timing */
  TIMING,
  /* will be executed once and put in TIMING after that */
  TIMING_READY,
  /* will be timing and killed after that */
  TIMING_ONCE,
  /* will be executed one single time */
  RUN_ONCE
};

/* The stask control block type */
typedef struct stasks_cb stasks_cb_t;

/* task function prototype */
typedef void (*stasks_fcn_t)(void);

extern volatile uint32_t systick_cntr;
extern int time_ms;
extern uint32_t time_s;

void stasks_init(void);
void stasks_run(void);
int stasks_add(stasks_fcn_t f, int status, int data, uint16_t count);
int stasks_add_once(stasks_fcn_t f, int status, int data, uint16_t count);
int stasks_replace_current(stasks_fcn_t f, int status, int data, uint16_t count);
stasks_cb_t * stasks_find(stasks_fcn_t f);
void stasks_killme(void);
int stasks_kill(stasks_fcn_t f);

#endif
