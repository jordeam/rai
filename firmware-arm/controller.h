#ifndef _controller_h
#define _controller_h

#include <stdint.h>

struct controller_params {
  float uk0, ak0, Kp, Ki, min, max;
};

typedef struct controller_params controller_t;

float controller_eval(controller_t * cont, float ref, float val);

/* data points to save in memory */
#define NUMPTS 720

/* period of pwm: TODO: se if define elsewhere */
#define PWM_PERIOD 1200

#define saturate(var, lower, upper) ((var < lower) ? lower : ((var > upper) ? upper : var))

void controller_i_pi(void);
void put_legs_high_Z(void);

#endif
