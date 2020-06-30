#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>

#include "encoder.h"

/* number of encoder measures to eval speed */
#define ENCODER_LOGS 16

static volatile int32_t encoder = 0;
static int32_t encoder_last = 0;
static int32_t encoder_0 = 0;
static int32_t data[ENCODER_LOGS];
static int data_i = 0;

static double theta0;

#define Delta_e ((2 * M_PI) / ENCODER_PULSES)

static float speed = 0;

pthread_mutex_t mut;

int32_t encoder_get(void) {
  pthread_mutex_lock(&mut);
  int32_t res = encoder - encoder_0; 
  pthread_mutex_unlock(&mut);
  return res;
}

void encoder_reset(void) {
  encoder_0 = encoder;
}

void encoder_eval(double theta) {
  pthread_mutex_lock(&mut);
  /* keep is used also as a timeout */
  int keep = 1;
  double next;
  /* encoder = (theta - theta0) * (ENCODER_PULSES / (2 * M_PI)); */
  while (keep) {
    if (theta >= (next = theta0 + Delta_e)) {
      encoder++;
      theta0 = next;
    }
    else if (theta <= (next = theta0 - Delta_e)) {
      encoder--;
      theta0 = next;
    }
    else
      keep = 0;
  }
  pthread_mutex_unlock(&mut);
}

/* Request a speed measure based in the actual encoder value. This function must be called periodically */
float encoder_speed(void) {
  pthread_mutex_lock(&mut);
  int32_t delta = encoder - encoder_last;
  speed += (delta - data[data_i]) * (1.0 / ENCODER_LOGS);
  data[data_i++] = delta;
  if (data_i >= ENCODER_LOGS)
    data_i = 0;
  encoder_last = encoder;
  pthread_mutex_unlock(&mut);
  return speed;
}

void encoder_init(double Theta0) {
  int i;
  theta0 = Theta0;
  encoder = encoder_last = Theta0 / Delta_e;
  for (i = 0; i < ENCODER_LOGS; i++) 
    data[i] = 0;
  if (pthread_mutex_init(&mut, NULL) != 0) {
    printf("ERROR: encoder_init: mutex_init failed\n");
    exit(1);
  }
}
