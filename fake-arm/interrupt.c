#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>

#include "state_phase.h"
#include "parameters.h"

/* interval between interruptions in us */
#define INTERVAL 1000
#define NUM_PHASES 5

/* Number of log lines */
#define NUMLOGS 2000

/* Number of log lines to call write function */
#define NLOGSPERWRITE 200

/* motor maximun speed rad/s */
#define OMEGA_MAX 314
#define INIT_ANG 40

/* Machanic transmission parameters */
#define r_m (15e-3 / 2)
#define r_1 (150e-3 / 2)
#define r_2 (15e-3 / 2)

#define VSTAT(x) ((x) ? "ON " : "OFF")

/* integrative constant for the control of interval between interrupts */
#define ki 0.1

/* counts each interruption */
uint32_t timer_i = 0;

/* natural expiration counter */
int expn_i = 0;

int V1 = 0, V2 = 0, V3 = 0, V4 = 0, V5 = 0, V6 = 0;

/* constants */
#define Demb 69e-3
#define A_e (M_PI * Demb * Demb / 4)

/* Input parameters */
int ventilator_run = 1;
float FIO2 = 0.3, VolINS = 0.4e-3, T_INS = 0.600, VolEXPF = 0.150e-3, T_EXPF = 0.4, T_EXPN = 1;

/* variables */
float x_O2, q_INS, kPOL = (r_m / r_1), x_EXPF, q_EXPF;
float omega_m = 0, x_T;
float theta_m = INIT_ANG;
float x_e = r_2 * (r_m / r_1) * INIT_ANG;

struct timespec t0;

void phase_reset(int phase_i);
void phase_O2(int phase_i);
void phase_air(int phase_i);
void phase_insp(int phase_i);
void phase_expf(int phase_i);

phase_fcn phase_table[5] = {phase_reset,
                            phase_O2,
                            phase_air,
                            phase_insp,
                            phase_expf};

struct log_block {
  int timer_i;
  uint8_t phase, v1, v2, v3, v4, v5, v6;
  float x_e;
};

typedef struct log_block log_block_t;

/* block of logs */
log_block_t data_log[NUMLOGS];

/* index for data_log */
int lpos = 0;
/* back position for data log */
int blpos = -1;

void phase_reset(int phase_i) {
  if (phase_i ==0) {
    /* Valves */
    V1 = V2 = V3 = V4 = 0;
    V5 = V6 = 1;
    if (x_e <= 0) {
      omega_m = 0;
    }
    else
      omega_m = -OMEGA_MAX;
    /* reset expn_i counter to timerize natural expiration */
    expn_i = 0;
    phase_i_inc();
  }
  /* phase */
  if (x_e <= 0) {
    omega_m = 0;
    if (ventilator_run) {
      if (FIO2 <= 0.22)
        phase_set(2 /* air */);
      else 
        phase_next();
    }
    /* else stay in phase 1 */
  }
  else
    omega_m = -OMEGA_MAX;
}

void phase_O2(int phase_i) {
  /* entering phase */
  if (phase_i ==0) {
    /* Valves */
    V1 = V3 = V4 = V5 = 0;
    V2 = V6 = 1;  
    /* must synchronize reading of VolINS and FIO2 with other thread */
    x_T = VolINS/A_e;
    x_O2 = (FIO2 - 0.2) / 0.8 * x_T;
    phase_i_inc();
  }
  /* phase */
  omega_m = OMEGA_MAX;
  if (x_e >= x_O2)
    phase_next();
}

void phase_air(int phase_i) {
  /* entering phase */
  if (phase_i ==0) {
    V1 = V6 = 1;
    V2 = V3 = V4 = V5 = 0;
    x_T = VolINS/A_e;
    phase_i_inc();
  }
  /* phase */
  if (x_e >= x_T)
    /* stop motor */
    omega_m = 0;
  else
    omega_m = OMEGA_MAX;
  
  /* Natural expiration time, begins with phase 1 */
  /* must synchronize the reading of T_EXPN with other thread */
  if (expn_i * INTERVAL * 1e-6 > T_EXPN)
    phase_next();
}

void phase_insp(int phase_i) {
  /* entering phase */
  if (phase_i ==0) {
    V3 = 1;
    V1 = V2 = V4 = V5 = V6 = 0;
    /* must synchronize reading of VolINS and T_INS with other thread */
    q_INS = VolINS / T_INS;
    omega_m = -q_INS / (A_e * r_2 * kPOL);
    phase_i_inc();
  }
  /* phase */
  if (x_e <= 0) {
    omega_m = 0;
    if (T_EXPF > 0 && VolEXPF > 0)
      phase_next();
    else
      phase_set(0 /* reset position */);
  }
}

void phase_expf(int phase_i) {
  /* entering phase */
  if (phase_i ==0) {
    V1 = V2 = V3 = V5 = V6;
    V4 = 1;
    /* must synchronize the reading of VolEXP and T_EXPF with other thread */
    x_EXPF = VolEXPF / A_e;
    q_EXPF = VolEXPF / T_EXPF;
    omega_m = q_EXPF / (A_e * r_2 * kPOL);
    phase_i_inc();
  }
  if (x_e >= x_EXPF) {
    omega_m = 0;
    phase_next();
  }
}

void print_status(void) {
  struct timespec ta;
  double t1;
  clock_gettime(CLOCK_REALTIME, &ta);
  t1 = (ta.tv_sec - t0.tv_sec) + (ta.tv_nsec - t0.tv_nsec) * 1e-9;
  /* printf("%08.3f %1d  %1d  %1d  %1d  %1d  %1d  %1d  %07d %07.1f %07.3f\n", t1, phase, V1, V2, V3, V4, V5, V6, expn_i, omega_m, x_e); */
}

void write_data(void * data) {
  FILE *f = fopen("log1.dat", "w");
  int i = blpos, count;
  /* printf("Writing data timer_i=%d lpos=%d blpos=%d\n", timer_i, lpos, blpos); */
  for (count = 0; count < NUMLOGS && i != lpos; count++)  {
    fprintf (f, "%f %d %d %d %d %d %d %d 0 0 %f\n", (float) data_log[i].timer_i / INTERVAL, data_log[i].phase, data_log[i].v1, data_log[i].v2, data_log[i].v3, data_log[i].v4, data_log[i].v5, data_log[i].v6, data_log[i].x_e);
    i++;
    if (i >= NUMLOGS)
      i = 0;
  }
  if (count >= NUMLOGS)
    blpos += NLOGSPERWRITE;
  if (blpos >= NUMLOGS)
    blpos -= NUMLOGS;
  fclose(f);
  rename("log1.dat", "log.dat");
}

void state_equations(void) {
  static struct timespec told = { 0, 0 };
  struct timespec ta;
  double dt;
  clock_gettime(CLOCK_REALTIME, &ta);
  /* if it is the first time in this function, do nothing */
  if (told.tv_sec == 0) {
    told.tv_sec = ta.tv_sec;
    told.tv_nsec = ta.tv_nsec;
    return;
  }
  /* eval dt */
  dt = ta.tv_sec - told.tv_sec + (ta.tv_nsec - told.tv_nsec) * 1e-9;
  dt = 1e-3;
  theta_m += omega_m * dt;
  x_e = theta_m * r_2 * kPOL;

  /* Logger */
  /* log data each 5 ms */
  if (timer_i % 5 == 0) {
    data_log[lpos].timer_i = timer_i;
    data_log[lpos].phase = phase_get();
    data_log[lpos].v1 = V1;
    data_log[lpos].v2 = V2;
    data_log[lpos].v3 = V3;
    data_log[lpos].v4 = V4;
    data_log[lpos].v5 = V5;
    data_log[lpos].v6 = V6;
    data_log[lpos].x_e = x_e;
    lpos++;
    if (lpos >= NUMLOGS) 
      lpos = 0;
    if (blpos < 0)
      blpos = 0;
    if (lpos == blpos) {
      blpos++;
      if (blpos >= NUMLOGS)
        blpos = 0;
    }
    if (lpos % NLOGSPERWRITE == 0) {
      /* create thread to write data in file */
      pthread_t trd;
      pthread_create(&trd, NULL, (void*) write_data, NULL);
      pthread_detach(trd);
    }
  }
}

void init_interrupt(void) {
  /* print_status header */
  printf("# t  phase  V1 V2 V3 V4 V5 V6 expn_i omega_m x_e\n");
  phase_init(0, print_status, NUM_PHASES, &phase_table[0], state_equations);
  state_equations();
  clock_gettime(CLOCK_REALTIME, &t0);
}

static void interrupt_1ms(int delta_t) {
  /* ARM code executed each 1ms goes here*/
  phase_exec();

  /* interrupt counter */
  timer_i++;

  /* timer for phases */
  expn_i++;
}

void * interval_code(int *max_counter) {
  int c = *max_counter;
  struct timespec tr, last_tp, tp, last_ti, ti;
  uint32_t delta_t, delta_ti;
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

    /* some log */
    if (--c == 0) {
      c = *max_counter;
      clock_gettime(CLOCK_REALTIME, &ti);
      delta_ti = ((ti.tv_sec - last_ti.tv_sec) * 1000000000 + ti.tv_nsec - last_ti.tv_nsec) / 1000;
      last_ti.tv_sec = ti.tv_sec;
      last_ti.tv_nsec = ti.tv_nsec;
      
      //      printf("Tick 1s: uk=%d delta_uk=%d ti = %ld.%09ld last_ti = %ld.%09ld delta_ti=%dus\n", uk, delta_uk, ti.tv_sec, ti.tv_nsec, last_ti.tv_sec, ti.tv_nsec, delta_ti);
    }
  }
  return NULL;
}

