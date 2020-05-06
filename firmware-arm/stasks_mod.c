/* Scheduled tasks module */

#include <stdlib.h>
#include <string.h>
#include "stm32f4xx.h"
#include "pins.h"

#include "stasks_mod.h"

/* the time counter in milli seconds */
int time_ms;

/* the time in seconds */
uint32_t time_s;

/* Magic number to know if it is a power up reset */
#define MAGIC 0xa56b793c
static volatile uint32_t magic;

volatile uint32_t systick_cntr = 0;

/* stasks control blocks */
stasks_cb_t stasks_cb[STASKSNUM];

/* stasks current control block index */
static int stasks_i;

/* stasks current control block pointer */
static stasks_cb_t * stasks_ctcb;

static void stasks_process_timers(void) {
  int i;
  stasks_cb_t * tcb = stasks_cb;
  for (i = 0; i < STASKSNUM; i++, tcb++) {
    if (tcb->f == NULL)
      continue;
    switch (tcb->status) {
    case READY:
    case BLOCKED:
    case TIMING_READY:
    case RUN_ONCE:
      /* do nothing */
      break;
    case TIMING:
      /* will repeat always, reload by data, which must be 16 bits only */
      if (tcb->count > 0)
        tcb->count--;
      if (tcb->count == 0)
        tcb->status = TIMING_READY;
      break;
    case TIMING_ONCE:
      if (tcb->count > 0)
        tcb->count--;
      if (tcb->count == 0)
        tcb->status = RUN_ONCE;
      break;
    }
  }
}

/*
  Kill current task by moving all TCBs from next to the current TCB position, one position up.
*/
static void stasks_kill_current(void) {
  stasks_cb_t * tcb = stasks_ctcb;
  int i = stasks_i;
  /* verify if it is not the last task */
  if (stasks_i == STASKSNUM - 1) {
    /* just clean this position and returns */
    stasks_ctcb->f = NULL;
  }
  else {
    /* it is not the last position, we need to copy the blocks up */
    while ((tcb+1)->f && i < STASKSNUM) {
      // char * temp_ptr = (char *)(tcb);
      //char * temp_ptr1 = (char *)(tcb+1);
      memcpy(tcb, tcb+1, sizeof(stasks_cb_t));
      // sizeof(stasks_cb_t);
      // memcpy(temp_ptr, temp_ptr1, sizeof(stasks_cb_t));
      tcb++;
      i++;
    }
    tcb->f = NULL;
  }
}

/*
  Process current task, returns 0 if ok, -1 if it were killed.
*/
static int stasks_process_current(void) {
  void (*f)(void) = stasks_ctcb->f;
  if (f == NULL)
    return 0;
  switch (stasks_ctcb->status) {
  case READY:
  case RUN_ONCE:
  case TIMING_READY:
    f();
    break;
  case KILLED:
  case BLOCKED:
  case TIMING:
  case TIMING_ONCE:
    break;
  }
  switch (stasks_ctcb->status) {
  case RUN_ONCE:
  case KILLED:
    stasks_kill_current();
    return -1;
  case TIMING_READY:
    stasks_ctcb->status = TIMING;
    stasks_ctcb->count = stasks_ctcb->data;
    break;
  case READY:
  case BLOCKED:
  case TIMING:
  case TIMING_ONCE:
    break;
  }
  return 0;
}

void stasks_init(void) {
  int i;
  for (i=0; i < STASKSNUM; i++) {
    stasks_cb[i].f = NULL;
  }
  stasks_i = 0;
  stasks_ctcb = stasks_cb;
  if (magic != MAGIC) {
    /* MAYBE it is a power on reset */
    magic = MAGIC;
    time_s = 0;
    time_ms = 0;
  }
  systick_cntr = 0;
}

/*
  Include a new stask in the array of stasks. Return 0 if ok, 1 if stasks array is full.
*/
int stasks_add(stasks_fcn_t f, int status, int data, uint16_t count) {
  /* find an empty tcb */
  int i;
  for (i=0; i < STASKSNUM; i++)
    if (stasks_cb[i].f == NULL) {
      stasks_cb[i].status = status;
      stasks_cb[i].data = data;
      stasks_cb[i].count = count;
      stasks_cb[i].f = f;
      return 0;
    }
  /* if code gets here, the array is full */
  return 1;
}

/*
  Gets the tcb of a task with the same address of function f, if it is there, returns null otherwise.
*/
stasks_cb_t * stasks_find(stasks_fcn_t f) {
  int i;
  for (i=0; i < STASKSNUM; i++)
    if (stasks_cb[i].f == f)
      return &stasks_cb[i];
  return NULL;
}

/*
  Include a new stask in the array of stasks only if it is not already there. Return 0 if ok, 1 if stasks array is full.
*/
int stasks_add_once(stasks_fcn_t f, int status, int data, uint16_t count) {
  /* find an empty tcb */
  int i;
  stasks_cb_t * tcb = stasks_find(f);
  if (tcb == NULL)
    for (i=0; i < STASKSNUM; i++)
      if (stasks_cb[i].f == NULL) {
        tcb = &stasks_cb[i];
        break;
      }
  if (tcb == NULL)
    return 1;
  tcb->f = f;
  tcb->status = status;
  tcb->data = data;
  tcb->count = count;
  /* success */
  return 0;
}

/*
  Replaces the current task by this new one.
*/
int stasks_replace_current(stasks_fcn_t f, int status, int data, uint16_t count) {
  stasks_ctcb->f = f;
  stasks_ctcb->status = status;
  stasks_ctcb->data = data;
  stasks_ctcb->count = count;
  /* success */
  return 0;
}

/*
  Kill the current task, visible from the user function.
*/
void stasks_killme(void) {
  stasks_ctcb->status = KILLED;
}

/*
  Kills a task by searching its function address.
  Returns 1 if success 0 otherwise.
*/
int stasks_kill(stasks_fcn_t f) {
  stasks_cb_t * tcb;
  tcb = stasks_find(f);
  if (tcb) {
    tcb->status = KILLED;
    return 1;
  }
  else
    return 0;
}


void stasks_run(void) {
  if (systick_cntr > 0) {
    /* turn on SIGI pin if it is in the begining of process */
    if (stasks_i == 0) {
      /* increments global time counter */
      time_ms++;
      if (time_ms >= 1000) {
        time_ms = 0;
        time_s++;
      }
      /* HIGH(SIGI); */
      stasks_process_timers();
    }
    /* process all tasks control blocks */
    /* Process the next stask and returns */

    /* Verify if it is empty */
    while (stasks_ctcb->f == NULL) {
      stasks_i++;
      stasks_ctcb++;
      if (stasks_i >= STASKSNUM) {
        /* resets index and tcb pointer */
        stasks_i = 0;
        stasks_ctcb = stasks_cb;
        /* Decrement systick_cntr */
        __disable_irq();
        systick_cntr--;
        __enable_irq();
        /* signals the end of stasks process */
        /* LOW(SIGI); */
        return;
      }
    }
    if (stasks_process_current() == 0) {
      /* increment stasks_i only if last task were not killed */
      /* point to next stasks_cb */
      stasks_i++;
      stasks_ctcb++;
      if (stasks_i >= STASKSNUM) {
        stasks_i = 0;
        stasks_ctcb = stasks_cb;
        /* Decrement systick_cntr */
        __disable_irq();
        systick_cntr--;
        __enable_irq();
        /* signals the end of stasks process */
        /* LOW(SIGI); */
      }
    }
  }
}
