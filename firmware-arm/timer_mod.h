#ifndef _timer_mod_h
#define _timer_mod_h

void timer1_set_fcn(void (*fcn)(void));
void timer3_set_fcn(void (*fcn)(void));

void TIM3_init(void);
void TIM4_init(void);
void TIM1_init(void);

#define Tpwm 0.0001f

extern uint16_t pwm_period;

#endif
