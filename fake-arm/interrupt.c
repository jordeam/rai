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
#include "encoder.h"

/* interval between interruptions in us */
#define INTERVAL 1000
#define NUM_PHASES 5

#define VALVE_OFF_INT 10
#define VALVE_ON_INT 10

/* motor maximun speed rad/s */
#define OMEGA_MAX 314
/* minimum speed to be considered stopped */
#define OMEGA_E_MIN 3

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
float k_p = 0.2;
#define TelMAX 1.0
float omega_ref = 0, omega_max = 0, x_ref = 0;
/* speed measured by encoder */
float omega_e = 0;
float Tel = 0;

/* Encoder speed constant */
#define k_es ((2 * M_PI) / (INTERVAL * 1e-6 * ENCODER_PULSES))

enum control_mode {control_speed, control_position, control_torque};

enum control_mode control_mode = control_speed;

sttmach_t phases;

void * phase_reset(sttmach_t *self);
void * phase_O2(sttmach_t *self);
void * phase_air(sttmach_t *self);
void * phase_inspiration(sttmach_t *self);
void * phase_forced_expiration(sttmach_t *self);

/* vars to be evaluated ony once per control cycle */
float x_enc = 0, x_con = 0;
static int32_t encoder;

/* phases' flags */
static int phase_eval_vars = 0;

/*
 * Phase Calib
 */

/* First use of cylinder, go to position 0 and resets encoder */
void * subphase_calib_resets_encoder(sttmach_t *self) {
  if (self->i == 0) {
    control_mode = control_speed;
    omega_ref = -0.2 * OMEGA_MAX;
  }
  else if ((fabsf(omega_e) < OMEGA_E_MIN || omega_e > 0) && self->i > 50) {
    /* reset encoder */
    encoder_set(0);
    omega_ref = 0;
    if (ventilator_run)
      return phase_reset;
    else
      return subphase_calib_resets_encoder;
    }
  return subphase_calib_resets_encoder;
}

void * subphase_calib_open_valves(sttmach_t *self) {
  if (self->i == 0) {
    V5 = V6 = 1;
    control_mode = control_speed;
    omega_ref = 0;
  }
  else if (self->i >= VALVE_ON_INT) 
    return subphase_calib_resets_encoder;
  return subphase_calib_open_valves;
}

void * subphase_calib_close_valves(sttmach_t *self) {
  if (self->i == 0) {
    V1 = V2 = V3 = V4 = 0;
    control_mode = control_speed;
    omega_ref = 0;
  }
  else if (self->i >= VALVE_OFF_INT) 
    return subphase_calib_open_valves;
  return subphase_calib_close_valves;
}

/* First use of cylinder, go to position 0 and resets encoder */
void * phase_calib(sttmach_t *self) {
  phases.id = 5;
  return subphase_calib_close_valves(self);
}

/*
 * Phase Reset
 */
void * subphase_reset_go_0(sttmach_t *self) {
  static int timer_x0;
  if (self->i == 0) {
    control_mode = control_position;
    omega_max = -OMEGA_MAX;
    x_ref = -2e-3;
    timer_x0 = 0;
  }
  else if (timer_x0++ > 20) {
    if (omega_e >= 0 && x_enc < 0.5e-3) {
      omega_max = 0;
      /* resets encoder */
      encoder_set(0);
      if (ventilator_run) {
        if (FIO2 <= 0.22)
          return phase_air;
        else
          /* go to next state */
          return phase_O2;
      }
      else
        /* else go to calib phase */
        return phase_calib;
    }
    else
      /* reset expn_i counter to timerize natural expiration */
      expn_i = 0;
  }
  /* stay in phase_reset */
  return subphase_reset_go_0;
}

void * subphase_reset_open_valves(sttmach_t *self) {
  if (self->i == 0) {
    V5 = V6 = 1;
    control_mode = control_speed;
    omega_ref = 0;
  }
  else if (self->i >= VALVE_ON_INT) 
    return subphase_reset_go_0;
  return subphase_reset_open_valves;
}

void * subphase_reset_close_valves(sttmach_t *self) {
  if (self->i == 0) {
    V1 = V2 = V3 = V4 = 0;
    control_mode = control_speed;
    omega_ref = 0;
  }
  else if (self->i >= VALVE_OFF_INT) 
    return subphase_reset_open_valves;
  return subphase_reset_close_valves;
}
  
void * phase_reset(sttmach_t *self) {
  /* return phase number */
  phases.id = 0;
  return subphase_reset_close_valves(self);
}

void * phase_O2(sttmach_t *self) {
  if (self->i == 0) {
    /* return phase number */
    phases.id = 1;
    x_T = VolINS/A_e;
    x_ref = (FIO2 - 0.2) / 0.8 * x_T;
  }
  if (self->i < VALVE_OFF_INT) {
    /* delay for valves to get closed */
    V1 = V3 = V4 = V5 = 0;
    omega_max = 0;
  }
  else if (self->i < VALVE_OFF_INT + VALVE_ON_INT) {
    /* Valves */
    V2 = V6 = 1;
    omega_max = 0;
  }
  else {
    omega_max = OMEGA_MAX;
    if (x_con >= x_ref) {
      omega_max = 0;
      return phase_air;
    }
  }
  /* stay in this phase */
  return phase_O2;
}

void * phase_air(sttmach_t *self) {
  static int count = 0;
  if (self->i == 0) {
    /* return phase number */
    phases.id = 2;
    x_ref = x_T = VolINS/A_e;
    count++;
    omega_max = 0;
  }
  if (self->i < VALVE_OFF_INT) {
    V2 = V3 = V4 = V5 = 0;
    omega_max = 0;
  }
  else if (self->i < VALVE_OFF_INT + VALVE_ON_INT) {
    if (x_con < x_T)
      V1 = V6 = 1;
  }
  else {
    if (x_con >= x_T) {
      /* stop motor */
      omega_max = 0;
      /* Close air valve */
      V1 = 0;
      if (count == 1)
        /* go to next phase if it is the first time of this phase */
        return phase_inspiration;
    }
    else
      omega_max = OMEGA_MAX;
    /* Natural expiration time, begins with phase 1 */
    /* must synchronize the reading of t_EXPN with other thread */
    if (count != 1 && expn_i * INTERVAL * 1e-6 > t_EXPN)
      return phase_inspiration;
  }
  /* stay in this phase */
  return phase_air;
}

void * phase_inspiration(sttmach_t *self) {
  static int count = 0;
  if (self->i == 0) {
    /* return phase number */
    phases.id = 3;
    count = 0;
    phase_eval_vars = 1;
  }
  if (self->i < VALVE_OFF_INT) {
    V1 = V2 = V4 = V5 = V6 = 0;
    omega_max = 0;
  }
  else if (self->i < VALVE_OFF_INT + VALVE_ON_INT) {
    V3 = 1;
  }
  else {
    if (phase_eval_vars) {
      phase_eval_vars = 0;
      q_INS = VolINS / t_INS;
      omega_max = -q_INS / (A_e * r_3 * kPOL);
      x_ref = 0;
    }
    if (x_con <= 0 || (omega_e > 0 && count > t_INS * (1.0 / (2 * INTERVAL *1e-6)))) {
      omega_max = 0;
      /* verify if a forced expiration is programed */
      if (t_EXPF > 0 && VolEXPF > 0)
        return phase_forced_expiration;
      else
        return phase_reset /* reset position */;
    }
  }
  count++;
  return phase_inspiration;
}

void * phase_forced_expiration(sttmach_t *self) {
  if (self->i == 0) {
    /* state number */
    phases.id = 4;
    phase_eval_vars = 1;
  }
  else if (self->i < VALVE_OFF_INT) {
    V1 = V2 = V3 = V5 = V6 = 0;
    omega_max = 0;
  }
  else if (self->i < VALVE_OFF_INT + VALVE_ON_INT) {
    V4 = 1;
  }
  else {
    if (phase_eval_vars) {
      phase_eval_vars = 0;
      /* must synchronize the reading of VolEXP and t_EXPF with other thread */
      x_ref = VolEXPF / A_e;
      q_EXPF = VolEXPF / t_EXPF;
      omega_max = q_EXPF / (A_e * r_3 * kPOL);
    }
    if (x_con >= x_ref - 1e-3) {
      omega_max = 0;
      return phase_reset;
    }
  }
  /* stay in this phase */
  return phase_forced_expiration;
}

/* controller */
void control_fcn(float x, float omega) {
  /* position control loop */
  switch (control_mode) {
  case control_position:
    /* position control loop */
    omega_ref = sign(x_ref - x) * sqrt(2 * TelMAX * (r_2 / (r_3 * r_1 * J_eq)) * fabsf(x_ref - x));
    if (omega_max > 0)
      omega_ref = saturate(omega_ref, omega_max, -OMEGA_MAX);
    else if (omega_max < 0)
      omega_ref = saturate(omega_ref, OMEGA_MAX, omega_max);
    else
      omega_ref = 0;
    break;
  case control_speed:
    omega_ref = saturate(omega_ref, OMEGA_MAX, -OMEGA_MAX);    
    break;
  case control_torque:
    break;
  }
  
  switch (control_mode) {
  case control_position:
  case control_speed:
    /* angular speed control loop */
    Tel = k_p * (omega_ref - omega);
  case control_torque:
    Tel = saturate(Tel, TelMAX, -TelMAX);
    Tel_set(Tel);
  }
}

static void interrupt_1ms(int delta_t) {
  /* ARM code executed each 1ms goes here*/
  /* encoder access */
  encoder = encoder_get();
  omega_e = k_es * encoder_speed(encoder);
  x_enc = (2 * M_PI * r_3 * kPOL / ENCODER_PULSES) * encoder;
  /* choose between real position get_x_e() and position givem by encoder x_enc */
  /* x_con = get_x_e(); */
  x_con = x_enc;
  /* eval state machine */
  sttmach_exec(&phases);
  /* controllers theirselves: choose between real position get_x_e() and position givem by encoder x_enc and real speed omega_m or encoder speed omega_e */
  control_fcn(x_con, omega_e);
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
  sttmach_init(&phases, phase_calib);

  pthread_t trd;
  int inter_data = INTERVAL;
  pthread_create(&trd, NULL, (void*) interval_code, &inter_data);
  pthread_detach(trd);
}

