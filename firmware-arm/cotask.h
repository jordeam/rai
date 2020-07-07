#ifndef _cotask_h
#define _cotask_h

#include <stdint.h>

#define COTASKNUM 30

/* The stask control block */
struct cotask_cb {
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
typedef struct cotask_cb cotask_cb_t;

/* task function prototype */
typedef void (*cotask_fcn_t)(void);

extern volatile uint32_t systick_cntr;
extern int time_ms;
extern uint32_t time_s;

void cotask_init(void);
void cotask_run(void);
int cotask_add(cotask_fcn_t f, int status, int data, uint16_t count);
int cotask_add_once(cotask_fcn_t f, int status, int data, uint16_t count);
int cotask_replace_current(cotask_fcn_t f, int status, int data, uint16_t count);
cotask_cb_t * cotask_find(cotask_fcn_t f);
void cotask_killme(void);
int cotask_kill(cotask_fcn_t f);

#endif
