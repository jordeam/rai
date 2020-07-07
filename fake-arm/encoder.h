#ifndef _encoder_h
#define _encoder_h

#include <stdint.h>

#define ENCODER_PULSES 600

int32_t encoder_get(void);
void encoder_reset(void);

#ifndef ARM
void encoder_eval(float theta);
#endif

float encoder_speed(void);
void encoder_init(float Theta0);

#endif
