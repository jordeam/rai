#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>

#include "mymath.h"
#include "awu_controller.h"
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
float x_T, x_O2, q_INS, q_EXPF;


/*
 * Control parameters and variables
 */
#define TelMAX 1.0
#define k_p 0.1
#define k_i (0.10f * INTERVAL * 1e-6f)
/* Speed controller */
awu_controller_t omega_cont = { 0, 0, k_p, k_i, TelMAX, -TelMAX};
float omega_ref = 0, omega_max = 0, x_ref = 0;
/* speed measured by encoder */
float omega_e = 0;
float Tel = 0;

/* Encoder speed constant */
#define k_es ((2 * M_PI) / (INTERVAL * 1e-6 * ENCODER_PULSES))

enum control_mode {control_speed, control_position, control_torque};

enum control_mode control_mode = control_speed;

sttmach_t phases;

void phase_reset(sttmach_t *self);
void phase_O2(sttmach_t *self);
void phase_air(sttmach_t *self);
void phase_inspiration(sttmach_t *self);
void phase_forced_expiration(sttmach_t *self);

/* vars to be evaluated ony once per control cycle */
float x_enc = 0, x_con = 0;
static int32_t encoder;
float rho_e;

/*
 * Phase Calib
 * First use of cylinder, go to position 0 and resets encoder
 */
void subphase_calib_resets_encoder(sttmach_t *self) {
  if (self->i == 0) {
    control_mode = control_speed;
    omega_ref = -0.2 * OMEGA_MAX;
  }
  else if ((fabsf(omega_e) < OMEGA_E_MIN || omega_e > 0) && self->i > 50) {
    /* reset encoder */
    encoder_set(0);
    omega_ref = 0;
    if (ventilator_run) {
      control_mode = control_speed;
      omega_ref = 0;
      /* delay for next substate */
      self->delay = VALVE_OFF_INT;
      self->state = phase_reset;
    }
  }
}

void subphase_calib_open_valves(sttmach_t *self) {
  V5 = V6 = 1;
  control_mode = control_speed;
  omega_ref = 0;
  self->delay = VALVE_ON_INT;
  /* next state */
  self->state = subphase_calib_resets_encoder;
}

/* First use of cylinder, go to position 0 and resets encoder */
void phase_calib(sttmach_t *self) {
  self->id = 5;
  subphase_calib_open_valves(self);
}

/*
 * Phase Reset
 */
void subphase_reset_go_0(sttmach_t *self) {
  static int timer_x0;
  if (self->i == 0) {
    control_mode = control_position;
    omega_max = -OMEGA_MAX;
    x_ref = -2e-3;
    timer_x0 = 0;
  }
  else if (timer_x0++ > 20) {
    if (omega_e >= 0 && x_enc < 0.5e-3) {
      /* close valves */
      V5 = 0;
      control_mode = control_speed;
      omega_ref = 0;
      self->delay = VALVE_OFF_INT;
      /* resets encoder */
      encoder_set(0);
      if (ventilator_run) {
        if (FIO2 <= 0.22)
          self->state = phase_air;
        else
          /* go to next state */
          self->state = phase_O2;
      }
      else
        /* else go to calib phase */
        self->state = phase_calib;
    }
    else
      /* reset expn_i counter to timerize natural expiration */
      expn_i = 0;
  }
}

void subphase_reset_open_valves(sttmach_t *self) {
  V5 = V6 = 1;
  control_mode = control_speed;
  omega_ref = 0;
  self->delay = VALVE_ON_INT;
  /* next state */
  self->state = subphase_reset_go_0;
}
  
void phase_reset(sttmach_t *self) {
  /* return phase number */
  self->id = 0;
  return subphase_reset_open_valves(self);
}

void subphase_O2_expand(sttmach_t *self) {
  if (self->i == 0) {
    self->id = 1;
    x_ref = x_O2;
    control_mode = control_position;
    omega_max = OMEGA_MAX;
  }
  else if (x_con >= x_ref) {
    self->delay = 20;
    /* next state */
    self->state = phase_air;
  }
}

void subphase_O2(sttmach_t *self) {
  if (x_con >= x_ref) {
    self->id = 7;
    /* Close Valves */
    V2 = 0;
    self->delay = VALVE_OFF_INT;
    /* next state */
    self->state = subphase_O2_expand;
  }
}

void subphase_O2_measure_pressure(sttmach_t *self) {
  control_mode = control_position;
  omega_max = OMEGA_MAX;
  x_ref = 3e-3;
  self->id = 6;
  if (x_con >= x_ref && self->i > 10 /* ms */) {
    /* eval new position */
    x_T = VolINS/A_e;
    x_O2 = (FIO2 - 0.2) / 0.8 * x_T;
    x_ref = rho_air / (rho_e + rho_air) * x_O2;
    /* next state */
    self->state = subphase_O2;
  }
}
  
void subphase_O2_open_valves(sttmach_t *self) {
  /* Valves */
  V2 = 1;
  control_mode = control_speed;
  omega_ref = 0;
  self->delay = VALVE_ON_INT;
  self->state = subphase_O2_measure_pressure;  
}

void phase_O2(sttmach_t *self) {
  x_ref = 0;
  self->id = 1;
  subphase_O2_open_valves(self);
}

/*
 * Air
 */
void subphase_air(sttmach_t *self) {
  static int count = 0;
  if (self->i == 0) {
    control_mode = control_position;
    x_ref = x_T = VolINS/A_e;
    count++;
    omega_max = OMEGA_MAX;
  }
  else {
    if (x_con >= x_T) {
      /* Close air valve */
      V1 = 0;
      if (count == 1) {
        /* go to next phase if it is the first time of this phase */
        control_mode = control_speed;
        omega_ref = 0;
        /* close natural expiration valve */
        V6 = 0;
        self->delay = VALVE_OFF_INT;
        /* next state */
        self->state = phase_inspiration;
      }
    }
    /* Natural expiration time, begins with phase 1 */
    /* must synchronize the reading of t_EXPN with other thread */
    if (count != 1 && expn_i * INTERVAL * 1e-6 > t_EXPN) {
      control_mode = control_speed;
      omega_ref = 0;
      /* close valves */
      V6 = 0;
      self->delay = VALVE_OFF_INT;
      /* next state */
      self->state = phase_inspiration;
    }
  }
}

void subphase_air_open_valves(sttmach_t *self) {
  V1 = 1;
  control_mode = control_speed;
  omega_ref = 0;
  self->delay = VALVE_ON_INT;
  /* next state */
  self->state = subphase_air;
}

void phase_air(sttmach_t *self) {
  /* phase number */
  self->id = 2;
  subphase_air_open_valves(self);
}

/*
 * Inspiration
 */
void subphase_inspiration(sttmach_t *self) {
  if (self->i == 0) {
    q_INS = VolINS / t_INS;
    omega_max = -q_INS / (A_e * r_ci * r_ei / r_ce);
    control_mode = control_position;
    x_ref = 0;
  }
  else if (x_con <= 0 || (omega_e > 0 && self->i > t_INS * (1.0 / (2 * INTERVAL *1e-6)))) {
    control_mode = control_speed;
    omega_ref = 0;
    /* close valves */
    V3 = 0;
    self->delay = VALVE_OFF_INT;
    /* verify if a forced expiration is programed */
    if (t_EXPF > 0 && VolEXPF > 0)
      /* next state */
      self->state = phase_forced_expiration;
    else
      /* next state */
      self->state = phase_reset /* reset position */;
  }
}

void subphase_inspiration_valves(sttmach_t *self) {
  V3 = 1;
  control_mode = control_speed;
  omega_ref = 0;
  self->delay = VALVE_ON_INT;
  /* next state */
  self->state = subphase_inspiration; 
}

void phase_inspiration(sttmach_t *self) {
  /* phase number */
  self->id = 3;
  /* next state */
  subphase_inspiration_valves(self);
}

/*
 * Forced Expiration
 */
void subphase_forced_expiration(sttmach_t *self) {
  if (self->i == 0) {
    /* must synchronize the reading of VolEXP and t_EXPF with other thread */
    x_ref = VolEXPF / A_e;
    q_EXPF = VolEXPF / t_EXPF;
    omega_max = q_EXPF / (A_e * r_ci * r_ei / r_ce);
    control_mode = control_position;
  }
  else if (x_con >= x_ref - 1e-3) {
    omega_ref = 0;
    control_mode = control_speed;
    /* close valves */
    V4 = 0;
    self->delay = VALVE_OFF_INT;
    /* next state */
    self->state = phase_reset;
  }
}

void subphase_forced_expiration_valves(sttmach_t *self) {
  V4 = 1;
  control_mode = control_speed;
  omega_ref = 0;
  self->delay = VALVE_ON_INT;
  /* next state */
  self->state = subphase_forced_expiration; 
}

void phase_forced_expiration(sttmach_t *self) {
  self->i = 4;
  subphase_forced_expiration_valves(self);
}

/* 
 * Controller
 */
void control_fcn(float x, float omega) {
  float diff;
  /* position control loop */
  switch (control_mode) {
  case control_position:
    /* position control loop */
    diff = 2 * (TelMAX - rho_e * (A_e * r_ci * (r_ei * r_pm) /(r_ce * r_ee))) * (r_ci * (r_ei * r_pm) /(r_ce * r_ee * J_eq)) * (x_ref - x);
    omega_ref = sign(diff) * sqrt(fabs(diff));
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
    Tel = awu_controller_eval(&omega_cont, omega_ref, omega);
  case control_torque:
    Tel = saturate(Tel, TelMAX, -TelMAX);
    Tel_set(Tel);
  }
}

static void interrupt_1ms(int delta_t) {
  /* ARM code executed each 1ms goes here*/
  /* encoder access */
  encoder = encoder_get();
  /* read pressure */
  rho_e = get_rho_e();
  omega_e = k_es * encoder_speed(encoder);
  x_enc = (2 * M_PI * r_ci * r_ei / (r_ce * ENCODER_PULSES)) * encoder;
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
  /* Valves init: close all */
  V1 = V2 = V3 = V4 = V5 = V6 = 0;
  
  state_equations_init();
  sttmach_init(&phases, phase_calib);

  pthread_t trd;
  int inter_data = INTERVAL;
  pthread_create(&trd, NULL, (void*) interval_code, &inter_data);
  pthread_detach(trd);
}

