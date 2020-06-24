#ifndef _encoder_h
#define _encoder_h

#include <stdint.h>

#define ENCODER_PULSES 600

int32_t encoder_get(void);
void encoder_set(int32_t newvalue);
void encoder_eval(double theta);
float encoder_speed(int32_t encoder);
void encoder_init(double Theta0);

#endif
