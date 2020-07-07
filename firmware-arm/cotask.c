/* Scheduled tasks module */

#include <stdlib.h>
#include <string.h>
#include "stm32f4xx.h"
#include "pins.h"

#include "cotask.h"

/* the time counter in milli seconds */
int time_ms;

/* the time in seconds */
uint32_t time_s;

/* Magic number to know if it is a power up reset */
#define MAGIC 0xa56b793c
static volatile uint32_t magic;

volatile uint32_t systick_cntr = 0;

/* cotask control blocks */
cotask_cb_t cotask_cb[COTASKNUM];

/* cotask current control block index */
static int cotask_i;

/* cotask current control block pointer */
static cotask_cb_t * cotask_ctcb;

static void cotask_process_timers(void) {
  int i;
  cotask_cb_t * tcb = cotask_cb;
  for (i = 0; i < COTASKNUM; i++, tcb++) {
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
static void cotask_kill_current(void) {
  cotask_cb_t * tcb = cotask_ctcb;
  int i = cotask_i;
  /* verify if it is not the last task */
  if (cotask_i == COTASKNUM - 1) {
    /* just clean this position and returns */
    cotask_ctcb->f = NULL;
  }
  else {
    /* it is not the last position, we need to copy the blocks up */
    while ((tcb+1)->f && i < COTASKNUM) {
      // char * temp_ptr = (char *)(tcb);
      //char * temp_ptr1 = (char *)(tcb+1);
      memcpy(tcb, tcb+1, sizeof(cotask_cb_t));
      // sizeof(cotask_cb_t);
      // memcpy(temp_ptr, temp_ptr1, sizeof(cotask_cb_t));
      tcb++;
      i++;
    }
    tcb->f = NULL;
  }
}

/*
  Process current task, returns 0 if ok, -1 if it were killed.
*/
static int cotask_process_current(void) {
  void (*f)(void) = cotask_ctcb->f;
  if (f == NULL)
    return 0;
  switch (cotask_ctcb->status) {
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
  switch (cotask_ctcb->status) {
  case RUN_ONCE:
  case KILLED:
    cotask_kill_current();
    return -1;
  case TIMING_READY:
    cotask_ctcb->status = TIMING;
    cotask_ctcb->count = cotask_ctcb->data;
    break;
  case READY:
  case BLOCKED:
  case TIMING:
  case TIMING_ONCE:
    break;
  }
  return 0;
}

void cotask_init(void) {
  int i;
  for (i=0; i < COTASKNUM; i++) {
    cotask_cb[i].f = NULL;
  }
  cotask_i = 0;
  cotask_ctcb = cotask_cb;
  if (magic != MAGIC) {
    /* MAYBE it is a power on reset */
    magic = MAGIC;
    time_s = 0;
    time_ms = 0;
  }
  systick_cntr = 0;
}

/*
  Include a new stask in the array of cotask. Return 0 if ok, 1 if cotask array is full.
*/
int cotask_add(cotask_fcn_t f, int status, int data, uint16_t count) {
  /* find an empty tcb */
  int i;
  for (i=0; i < COTASKNUM; i++)
    if (cotask_cb[i].f == NULL) {
      cotask_cb[i].status = status;
      cotask_cb[i].data = data;
      cotask_cb[i].count = count;
      cotask_cb[i].f = f;
      return 0;
    }
  /* if code gets here, the array is full */
  return 1;
}

/*
  Gets the tcb of a task with the same address of function f, if it is there, returns null otherwise.
*/
cotask_cb_t * cotask_find(cotask_fcn_t f) {
  int i;
  for (i=0; i < COTASKNUM; i++)
    if (cotask_cb[i].f == f)
      return &cotask_cb[i];
  return NULL;
}

/*
  Include a new stask in the array of cotask only if it is not already there. Return 0 if ok, 1 if cotask array is full.
*/
int cotask_add_once(cotask_fcn_t f, int status, int data, uint16_t count) {
  /* find an empty tcb */
  int i;
  cotask_cb_t * tcb = cotask_find(f);
  if (tcb == NULL)
    for (i=0; i < COTASKNUM; i++)
      if (cotask_cb[i].f == NULL) {
        tcb = &cotask_cb[i];
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
int cotask_replace_current(cotask_fcn_t f, int status, int data, uint16_t count) {
  cotask_ctcb->f = f;
  cotask_ctcb->status = status;
  cotask_ctcb->data = data;
  cotask_ctcb->count = count;
  /* success */
  return 0;
}

/*
  Kill the current task, visible from the user function.
*/
void cotask_killme(void) {
  cotask_ctcb->status = KILLED;
}

/*
  Kills a task by searching its function address.
  Returns 1 if success 0 otherwise.
*/
int cotask_kill(cotask_fcn_t f) {
  cotask_cb_t * tcb;
  tcb = cotask_find(f);
  if (tcb) {
    tcb->status = KILLED;
    return 1;
  }
  else
    return 0;
}


void cotask_run(void) {
  if (systick_cntr > 0) {
    /* turn on SIGI pin if it is in the begining of process */
    if (cotask_i == 0) {
      /* increments global time counter */
      time_ms++;
      if (time_ms >= 1000) {
        time_ms = 0;
        time_s++;
      }
      /* HIGH(SIGI); */
      cotask_process_timers();
    }
    /* process all tasks control blocks */
    /* Process the next stask and returns */

    /* Verify if it is empty */
    while (cotask_ctcb->f == NULL) {
      cotask_i++;
      cotask_ctcb++;
      if (cotask_i >= COTASKNUM) {
        /* resets index and tcb pointer */
        cotask_i = 0;
        cotask_ctcb = cotask_cb;
        /* Decrement systick_cntr */
        __disable_irq();
        systick_cntr--;
        __enable_irq();
        /* signals the end of cotask process */
        /* LOW(SIGI); */
        return;
      }
    }
    if (cotask_process_current() == 0) {
      /* increment cotask_i only if last task were not killed */
      /* point to next cotask_cb */
      cotask_i++;
      cotask_ctcb++;
      if (cotask_i >= COTASKNUM) {
        cotask_i = 0;
        cotask_ctcb = cotask_cb;
        /* Decrement systick_cntr */
        __disable_irq();
        systick_cntr--;
        __enable_irq();
        /* signals the end of cotask process */
        /* LOW(SIGI); */
      }
    }
  }
}
