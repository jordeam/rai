#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>

#include "sttmach.h"
#include "oper_parameters.h"
#include "state_equations.h"
#include "plant_parameters.h"

/* interval between interruptions in us */
#define INTERVAL 1000
#define NUM_PHASES 5

#define VALVE_OFF_INT 10
#define VALVE_ON_INT 10

/* motor maximun speed rad/s */
#define OMEGA_MAX 314

/* valve macro for debug only */
#define VSTAT(x) ((x) ? "ON " : "OFF")

/* integrative constant for the control of interval between interrupts */
#define ki 0.1

/* counts each interruption */
uint32_t timer_i = 0;

/* natural expiration counter */
int expn_i = 0;

/* valve states */
int V1 = 0, V2 = 0, V3 = 0, V4 = 0, V5 = 0, V6 = 0;

/* operational parameters */
int ventilator_run = 1;
float FIO2 = 0.3, VolINS = 0.4e-3, t_INS = 0.800, VolEXPF = 0.150e-3, t_EXPF = 0.4, t_EXPN = 0.8;

/* variables */
float x_T, q_INS, q_EXPF;

/*
 * Control parameters and variables
 */
float k_p = 1;
#define TelMAX 1.0
float omega_ref = 0, omega_max = 0, x_ref = 0;
float Tel = 0;

int phase_reset(int cur_state, int phase_i);
int phase_O2(int cur_state, int phase_i);
int phase_air(int cur_state, int phase_i);
int phase_inspiration(int cur_state, int phase_i);
int phase_forced_expiration(int cur_state, int phase_i);

sttmach_fcn phase_table[5] = {phase_reset,
                              phase_O2,
                              phase_air,
                              phase_inspiration,
                              phase_forced_expiration};

int phase_reset(int cur_state, int phase_i) {
  float x_e = get_x_e();
  x_ref = 0;
  if (phase_i < VALVE_OFF_INT) {
    /* 5ms for valves get closed? */
    /* Valves */
    V1 = V2 = V3 = V4 = 0;
    omega_max = 0;
  }
  else if (phase_i < VALVE_OFF_INT + VALVE_ON_INT) {
    /* more 5ms to valves open? */
    V5 = V6 = 1;
    omega_max = 0;
    /* reset expn_i counter to timerize natural expiration */
    expn_i = 0;
  }
  else if (x_e <= 0) {
    omega_max = 0;
    if (ventilator_run) {
      if (FIO2 <= 0.22)
        return 2 /* air */;
      else 
        return ++cur_state;
    }
    else {
      /* else stay in phase 1 */
      /* reset expn_i counter to timerize natural expiration */
      expn_i = 0;
    }
  }
  else
    omega_max = -OMEGA_MAX;
  return cur_state;
}

int phase_O2(int cur_state, int phase_i) {
  float x_e = get_x_e();
  if (phase_i == 0) {
    x_T = VolINS/A_e;
    x_ref = (FIO2 - 0.2) / 0.8 * x_T;
  }
  if (phase_i < VALVE_OFF_INT) {
    /* delay for valves to get closed */
    V1 = V3 = V4 = V5 = 0;
    omega_max = 0;
  }
  else if (phase_i < VALVE_OFF_INT + VALVE_ON_INT) {
    /* Valves */
    V2 = V6 = 1;
    omega_max = 0;
  }
  else {
    omega_max = OMEGA_MAX;
    if (x_e >= x_ref) {
      omega_max = 0;
      return ++cur_state;
    }
  }
  return cur_state;
}

int phase_air(int cur_state, int phase_i) {
  float x_e = get_x_e();
  int eval_vars;
  static int count = 0;
  if (phase_i == 0) {
    eval_vars = 1;
    count++;
  }
  if (phase_i < VALVE_OFF_INT) {
    V2 = V3 = V4 = V5 = 0;
    omega_max = 0;
  }
  else if (phase_i < VALVE_OFF_INT + VALVE_ON_INT) {
    if (x_e < x_T)
      V1 = V6 = 1;
  }
  else {
    if (eval_vars) {
      eval_vars = 0;
      x_ref = x_T = VolINS/A_e;
    }
    if (x_e >= x_T) {
      /* stop motor */
      omega_max = 0;
      /* Close air valve */
      V1 = 0;
      if (count == 1)
        /* go to next phase if it is the first time of this phase */
        return ++cur_state;
    }
    else
      omega_max = OMEGA_MAX;
    /* Natural expiration time, begins with phase 1 */
    /* must synchronize the reading of t_EXPN with other thread */
    if (count != 1 && expn_i * INTERVAL * 1e-6 > t_EXPN)
      return ++cur_state;
  }
  return cur_state;
}

int phase_inspiration(int cur_state, int phase_i) {
  float x_e = get_x_e();
  int eval_vars;
  if (phase_i == 0)
    eval_vars = 1;
  if (phase_i < VALVE_OFF_INT) {
    V1 = V2 = V4 = V5 = V6 = 0;
    omega_max = 0;
  }
  else if (phase_i < VALVE_OFF_INT + VALVE_ON_INT) {
    V3 = 1;
  }
  else {
    if (eval_vars) {
      eval_vars = 0;
      q_INS = VolINS / t_INS;
      omega_max = -q_INS / (A_e * r_3 * kPOL);
      x_ref = 0;
    }
    if (x_e <= 0) {
      omega_max = 0;
      /* verify if a forced expiration is programed */
      if (t_EXPF > 0 && VolEXPF > 0)
        return ++cur_state;
      else
        return 0 /* reset position */;
    }
  }
  return cur_state;
}

int phase_forced_expiration(int cur_state, int phase_i) {
  float x_e = get_x_e();
  int eval_vars;
  if (phase_i == 0)
    eval_vars = 1;
  if (phase_i < VALVE_OFF_INT) {
    V1 = V2 = V3 = V5 = V6 = 0;
    omega_max = 0;
  }
  else if (phase_i < VALVE_OFF_INT + VALVE_ON_INT) {
    V4 = 1;
  }
  else {
    if (eval_vars) {
      eval_vars = 0;
      /* must synchronize the reading of VolEXP and t_EXPF with other thread */
      x_ref = VolEXPF / A_e;
      q_EXPF = VolEXPF / t_EXPF;
      omega_max = q_EXPF / (A_e * r_3 * kPOL);
    }
    if (x_e >= x_ref - 1e-3) {
      omega_max = 0;
      return ++cur_state;
    }
  }
  return cur_state;
}

/* controller */
void control_fcn(void) {
  float x_e = get_x_e();
  float omega_m = get_omega_m();
  omega_ref = sign(x_ref - x_e) * (r_2 / (r_3 * r_1)) * sqrt(2 * TelMAX * fabsf(x_ref - x_e));
  if ((omega_max > 0 && omega_ref > omega_max) || (omega_max < 0 && omega_ref < omega_max))
    omega_ref = omega_max;
  Tel = k_p * (omega_ref - omega_m);
  Tel = saturate(Tel, TelMAX, -TelMAX);
  Tel_set(Tel);
}

static void interrupt_1ms(int delta_t) {
  /* ARM code executed each 1ms goes here*/
  sttmach_exec();
  control_fcn();
  /* interrupt counter */
  timer_i++;
  /* timer for natural expiration */
  expn_i++;
}

static void * interval_code(int *max_counter) {
  /* int c = *max_counter; */
  struct timespec tr, last_tp, tp, last_ti;
  uint32_t delta_t;
  int32_t uk = INTERVAL, delta_uk;
  
  clock_getres(CLOCK_REALTIME, &tr);
  clock_gettime(CLOCK_REALTIME, &last_tp);
  clock_gettime(CLOCK_REALTIME, &last_ti);
  printf("tr{tv_sec=%ld,tv_nsec=%ld}\n", tr.tv_sec, tr.tv_nsec);
  for (;;) {
    usleep(uk);
    clock_gettime(CLOCK_REALTIME, &tp);
    delta_t = ((tp.tv_sec - last_tp.tv_sec) * 1000000000 + tp.tv_nsec - last_tp.tv_nsec) / 1000;
    last_tp.tv_sec = tp.tv_sec;
    last_tp.tv_nsec = tp.tv_nsec;
    /* PI control to adjust time*/
    delta_uk = ki * (INTERVAL - (int)delta_t);
    /* saturate delta_uk */
    if (delta_uk > 200)
      delta_uk = 200;
    else if (delta_uk < -200)
      delta_uk = -200;
    uk += delta_uk;
    if (uk < 0)
      uk = 0;
 
    /* calls the interrupt */
    interrupt_1ms(delta_t);
  }
  return NULL;
}

void interrupt_init(void) {
  /* print_status header */
  printf("# init_interrupt\n");
  state_equations_init();
  sttmach_init(0, NUM_PHASES, &phase_table[0]);

  pthread_t trd;
  int inter_data = INTERVAL;
  pthread_create(&trd, NULL, (void*) interval_code, &inter_data);
  pthread_detach(trd);
}

