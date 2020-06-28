#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <sys/types.h> 
#include <string.h> 
#include <sys/wait.h> 
#include <errno.h>
#include <semaphore.h>

#include "sttmach.h"
#include "circbuf.h"
#include "oper_parameters.h"
#include "plant_parameters.h"
#include "encoder.h"

#define INIT_ANG 40

#define WRITE_DATA_INTERVAL 0.5

/* Logger */
/* Number of log lines */
#define NUMLOGS 2000

circbufrw_t datalog;

struct log_block {
  double t;
  uint8_t phase, v1, v2, v3, v4, v5, v6;
  float x_ref, x_e, x_enc;
  float omega_m, omega_ref, omega_e, Tel, rho_e, P_m;
};

typedef struct log_block log_block_t;

/* block of logs */
log_block_t data_log[NUMLOGS];

/* syncing gnuplot */
volatile sem_t sem;
static volatile double gnuplot_t;

/* Pipes */
int fd1[2];  // Used to store two ends of first pipe 
int fd2[2];  // Used to store two ends of second pipe 

/*
 *  State variables
 */
/* motor angular speed */
static double omega_m = 0;
/* motor angular position */
static double theta_m = INIT_ANG;
/* piston position */
static double x_e = ((r_ci * r_ei * r_pm) / (r_ce * r_ee) * INIT_ANG);
/* relative pressure inside cylinder in Pa */
static double rho_e = 0;

/*
 * Static parameters
 */

/* piston end of course constants */
/* position for F_a_MAX */
#define xneg -2e-3
static double F_a;
/* maximum value at xneg */
#define F_a_MAX 2000

/* time constant for O2 valve open */
#define tau_rho_O2 0.02

/* filter constant */
#define k 0.05

/*
 * Input variables
 */

/* motor electromagnetic torque, it is set outside */
static double Tel;

/* using external, be carefull */
extern int V1, V2, V3, V4, V5, V6;

/* using external to log */
extern float x_ref, x_enc;
extern float omega_ref;
extern float omega_e;
extern sttmach_t phases;

/*
 * Variables
 */

/* Average mechanical power of motor axis */
static double P_m;

/*
 * Accessor methods
 */
void Tel_set(float _Tel) {
  Tel = _Tel;
}

float get_omega_m(void) {
  return omega_m;
}

float get_x_e(void) {
  return x_e;
}

float get_rho_e(void) {
  return (float) rho_e;
}

/*
 * Logging
 */
void write_data(void * data) {
  double t = *((double *) data);
  FILE *f = fopen("log1.dat", "w");
  int lpos = datalog.wi;
  int i = datalog.ri, count;
  /* printf("Writing data t = %f lpos=%d blpos=%d numlogs=%d\n", t, lpos, i, numlogs); */
  for (count = 0; i != lpos; count++)  {
    fprintf (f, "%f %d %d %d %d %d %d %d %f %f %f %f %f %f %f %f %f\n",
             (float) data_log[i].t, data_log[i].phase, data_log[i].v1, data_log[i].v2, data_log[i].v3, data_log[i].v4, data_log[i].v5, data_log[i].v6,
             data_log[i].x_ref, data_log[i].x_e, data_log[i].x_enc,
             data_log[i].omega_m, data_log[i].omega_ref, data_log[i].omega_e,
             data_log[i].Tel, data_log[i].rho_e, data_log[i].P_m);
    i++;
    if (i >= NUMLOGS)
      i = 0;
  }
  fclose(f);
  rename("log1.dat", "log.dat");
  /* sync gnuplot */
  gnuplot_t = t;
  sem_post(&sem);
}

void state_equations(void * ignore) {
  struct timespec ta, t_0;
  double t, dt, t_old, t_log, t_write = 0;
  double d_omega_m, d_theta_m, d_rho_e;
  clock_gettime(CLOCK_REALTIME, &t_0);
  t_old = 0;
  t_log = -1;
  for (;; /* C for is cool */ usleep(200), t_old = t) {
    /* determine t, eval dt */
    clock_gettime(CLOCK_REALTIME, &ta);
    t = ta.tv_sec - t_0.tv_sec + (ta.tv_nsec - t_0.tv_nsec) * 1e-9;
    dt = t - t_old;
    if (end_time > 0 && t > end_time) {
      printf("INFO: finishing by end_time\n");
      exit(0);
    }
    /* End of piston 100 N at -2mm */
    F_a = (F_a_MAX / xneg) * x_e;
    F_a = saturate(F_a, F_a_MAX, 0);
    
    /* measuring motor mechanical power */
    P_m = (1 - k) * P_m + k * omega_m * Tel;
  
    /* state equations */
    d_omega_m = -(B_eq / J_eq) * omega_m + (1 / J_eq) * Tel + ((r_ci * r_ei * r_pm) / (r_ce * r_ee * J_eq)) * F_a + ((A_e * r_ci * r_ei * r_pm) / (r_ce * r_ee * J_eq)) * rho_e;
    /* fim de curso */
    if (x_e < 0) {
      if (d_omega_m < 0) d_omega_m = 0;
      if (omega_m < 0) omega_m *= 0.1;
    }
    d_theta_m = omega_m;
    if (V2)
      d_rho_e = (-rho_e + rho_O2) / tau_rho_O2;
    else if (V1 || V3 || V5 || V4)
      d_rho_e = -rho_e / tau_rho_O2;
    else {
      float d_x_e = d_theta_m * ((r_ci * r_ei * r_pm) / (r_ce * r_ee));
      float d_vol_e = d_x_e * A_e;
      float vol_e = x_e * A_e;
      if (fabs(vol_e) < 1e-6)
        d_rho_e = 0;
      else
        d_rho_e = - ((rho_e + rho_air) / vol_e) * d_vol_e;
    }
    /* refreshing state variables */
    omega_m += d_omega_m * dt;
    theta_m += d_theta_m * dt;
    rho_e += d_rho_e * dt;
    x_e = theta_m * ((r_ci * r_ei * r_pm) / (r_ce * r_ee));
    encoder_eval(theta_m * (r_pm / r_ee));
    /* Logger */
    /* log data each 5 ms */
  
    if (t_log < 0 || t - t_log >=  5e-3) {
      int lpos = datalog.wi;
      t_log = t;
      data_log[lpos].t = t;
      data_log[lpos].phase = phases.id;
      data_log[lpos].v1 = V1;
      data_log[lpos].v2 = V2;
      data_log[lpos].v3 = V3;
      data_log[lpos].v4 = V4;
      data_log[lpos].v5 = V5;
      data_log[lpos].v6 = V6;
      data_log[lpos].x_e = x_e;
      data_log[lpos].x_enc = x_enc;
      data_log[lpos].omega_m = omega_m;
      data_log[lpos].omega_ref = omega_ref;
      data_log[lpos].omega_e = omega_e;
      data_log[lpos].x_ref = x_ref;
      data_log[lpos].Tel = Tel;
      data_log[lpos].rho_e = rho_e;
      data_log[lpos].P_m = P_m;
      circbufrw_inc_wi(&datalog);
      if (t - t_write >= WRITE_DATA_INTERVAL) {
        /* time to write data to file */
        /* create thread to write data in file */
        pthread_t trd;
        pthread_create(&trd, NULL, (void*) write_data, &t);
        pthread_detach(trd);
        t_write = t_write + WRITE_DATA_INTERVAL;
      }
    }
  }
}

#define READ_END 0
#define WRITE_END 1

void * gnuplot_init(void) { 
  int p1[2] = {-1,-1},	/* parent -> child */
    p2[2] = {-1,-1};	/* child -> parent */
  pid_t childpid;

  if ( pipe(p1) < 0  ||  pipe(p2) < 0 ) {
    /* FATAL: cannot create pipe */
    /* close readpipe[0] & [1] if necessary */
    return 0;
  }

  if ((childpid = fork()) < 0) {
    /* FATAL: cannot fork child */
    perror("gnuplot_init: can not launch gnuplot");
  }
  else if (childpid == 0) {	/* in the child */
    printf("gnuplot_init: Entering child\n"); 
    close(p1[WRITE_END]);
    close(p2[READ_END]);
    /* read pipe */
    dup2(p1[READ_END], 0);
    close(p1[READ_END]);
    /* do not duplicate child standard ouput to write pipe: let it write on its standard output */
    /* dup2(p2[WRITE_END], 1); */
    close(p2[WRITE_END]);
    
    /* do child stuff */
    char * args[] = {"gnuplot", NULL};
    printf("INFO: launching gnuplot\n");
    int res = execv("/usr/bin/gnuplot", args);
    if (res != 0) {
      perror("gnuplot_init: execv");
      exit(errno);
    }
    /* will never get here */
    return NULL;
  }
  /* else  */
  /* in the parent */
  close(p1[READ_END]);
  close(p2[WRITE_END]);
  sleep(1);
  FILE * f = fdopen(p1[WRITE_END], "w");
  fprintf(f, "set term qt size 1600,500\nload 'log.gp'\n");
  fflush(f);
  printf("INFO: gnuplot_init: sent plot to gnuplot\n");
  for(;;) {
    /* wait semaphore */
    sem_wait(&sem);
    fprintf(f, "set xrange [%f:%f]\nreplot\n", (gnuplot_t < 8.0) ? 0 : gnuplot_t - 8, gnuplot_t);
    fflush(f);
  }
  return NULL;
}

void state_equations_init(void) {
  /* to synchronize gnuplot */
  if (sem_init(&sem, 0, 0) == -1)
    perror("sem_init");
  /* data logger buffer */
  circbufrw_init(&datalog, NUMLOGS);
  /* encoder emulation */
  encoder_init(theta_m);
  /* a new thread to control gnuplot */
  pthread_t trd;
  pthread_create(&trd, NULL, (void*) gnuplot_init, NULL);
  pthread_detach(trd);

  pthread_create(&trd, NULL, (void*) state_equations, NULL);
  pthread_detach(trd);
}
