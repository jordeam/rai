#include <math.h>
#include "spi_mod.h"
#include "pbit.h"
#include "pins.h"
#include "gpio_mod.h"
#include "controller.h"
#include "ad_mod.h"
#include "timer_mod.h"
#include "sin-cos.h"

#define A120 2.0943951023931953f
#define A30 0.523598775598f
#define M_sqrt_2_3 0.816496580927726f
#define M_sqrt_1_3 0.577350269189f
#define M_sqrt3_2 0.866025403785f
#define M_2PI 6.28318530718f
#define M_sqrt3  1.73205080757f
#define M_sqrt2 1.41421356237f
#define M_sqrt_1_3 0.577350269189f

#define sqr(x) ((x)*(x))

#define IMAX 4

/* from main: */
extern int opmode;

/* from ad_mod: */
extern int j_i0, j_i1, j_i2, j_i3, j_ia, j_ib;

/* from timer_mod.c */
extern uint16_t pwm_period;

/* offsets */
float offset_st1 = 0, offset_st2 = 0, offset_st3 = 0, offset_st4 = 0, offset_sc1 = 0, offset_sc2 = 0, offset_sc3 = 0, offset_sc4 = 0;

/* AD values */
int16_t ad_values[8];

static uint8_t ch1state = 0, ch2state = 0, ch3state = 0;

/* sensors */
/* Fator para converter o valor de entrada do sensor em nominal */
float fest1 = ( 1.0566f * 0.088751166f), fest2 = ( 1.0566f * 0.088957899f), fsst1 = 5.85f, fsst2 = 5.85f, fest3 = 0.093587904f, fest4 = 0.090277007f, fsst3 = 5.85f, fsst4 = 5.85;
float fesc1 = 0.000593453f, fesc2 = 0.000606088f, fssc1 = 204.75f, fssc2 = 204.75f, fesc3 = 0.002283129f, fesc4 = 0.002185073f, fssc3 = 136.5f, fssc4 = 136.5f;

/* counts how many times timer 1 IRQ occurred */
extern int tim1_irq_count;

/* current controller parameters and variables */
controller_t i_cont = { 0, 0, /* Kp */ 10.0f, /* Ki */ 1.0f * Tpwm, 0, 1.0f};

/* Voltage controller parameters */
controller_t v_cont = { 0, 0, /* Kp */ 0.0051f, /* Ki */ 10.1f * Tpwm, -IMAX, IMAX};

float Vbus = 300;
float Iref;

/* Measures */
float Vab, Vbc, Vca, ia, Vmod, Vq, theta = 0, omega = 0.5 * (2 * M_PI * 60);

/* Debug */
float theta2 = M_PI, omega2 = 377.f;

/* telemetry */
// float speed_acq[NUMPTS];
// float Ik_acq[NUMPTS];
// float I_ref_acq[NUMPTS];
// float Ik1_acq[NUMPTS];
// int dc_acq[NUMPTS];

static inline void leg1_low_switch_PWM(void) {
  if (ch1state == 0) {
    GPIO_config_mode(PWM1N, GPIO_MODE_AF);
    GPIO_config_af(PWM1N, 1);
    ch1state = 1;
  }
}

static inline void leg2_low_switch_PWM(void) {
  if (ch2state == 0) {
    /* CH2 must be put into PWM AF */
    GPIO_config_mode(PWM2N, GPIO_MODE_AF);
    GPIO_config_af(PWM2N, 1);
    ch2state = 1;
  }
}

static inline void leg3_low_switch_PWM(void) {
  if (ch3state == 0) {
    /* CH3 must be put into PWM AF */
    GPIO_config_mode(PWM3N, GPIO_MODE_AF);
    GPIO_config_af(PWM3N, 1);
    ch3state = 1;
  }
}

static inline void leg1_low_switch_OFF(void) {
  if (ch1state) {
    GPIO_config_mode(PWM1N, GPIO_MODE_OUT);
    LOW(PWM1N);
    ch1state = 0;
  }
}

static inline void leg2_low_switch_OFF(void) {
  if (ch2state) {
    GPIO_config_mode(PWM2N, GPIO_MODE_OUT);
    LOW(PWM2N);
    ch2state = 0;
  }
}

static inline void leg3_low_switch_OFF(void) {
  if (ch3state) {
    GPIO_config_mode(PWM3N, GPIO_MODE_OUT);
    LOW(PWM3N);
    ch3state = 0;
  }
}

/* Put all legs in high impedance */
void put_legs_high_Z(void) {
  /* phase A is in high impedance */
  leg1_low_switch_OFF();
  TIM1->CCR1 = 0;
  /* phase B is in high impedance */
  leg2_low_switch_OFF();
  TIM1->CCR2 = 0;
  /* Phase C is off */
  leg3_low_switch_OFF();
  TIM1->CCR3 = 0;
}

/* Voltage controller, current reference as output  */
float controller_eval(controller_t * cont, float ref, float val) {
  float ak;
  float uk;
  float err = ref - val;
  ak = cont->Kp * err;
  uk = ak - cont->ak0 + cont->Ki * err + cont->uk0;
  uk = saturate(uk, cont->min, cont->max);
  cont->ak0 = saturate(ak, cont->min, cont->max);
  cont->uk0 = uk;
  return uk;
}

void abc_to_alphabeta(float a, float b, float c, float *alpha, float *beta, float *zero) {
  *alpha = M_sqrt_2_3 * (a - 0.5 * b - 0.5 * c);
  *beta = M_sqrt_2_3 * (M_sqrt3_2 * b - M_sqrt3_2 * c);
  *zero = M_sqrt_1_3 * (a + b + c);
}

void alphabeta_to_dq(float alpha, float beta, float *d, float *q, float theta) {
  *d = alpha * cos_lookup(theta) + beta * sin_lookup(theta);
  *q = -alpha * sin_lookup(theta) + beta * cos_lookup(theta);
}

/* make one PLL step, returns TRUE if converged, FALSE otherwise */
int PLL(float Vab, float Vbc, float Vca, float * theta, float * omega, float * vq) {
  static float vd0 = 0;
  float vd;
  const float Kp = 0.10f;
  const float Ki = 5.10f;
  float alpha, beta, zero;
  abc_to_alphabeta(Vab, Vbc, Vca, &alpha, &beta, &zero);
  alphabeta_to_dq(alpha, beta, &vd, vq, *theta);
  *omega += Kp * (vd - vd0) + Ki * Tpwm * vd;
  /* to converge faster */
  *omega = saturate(*omega, -450, 450);
  vd0 = vd;
  *theta += *omega * Tpwm;
  while (*theta > M_2PI) *theta -= M_2PI;
  while (*theta < M_2PI) *theta += M_2PI;
  return (vd < 1 && vd > -1);
}

/* This function is called each PWM cycle, which is determined by TIM1 */
void controller_i_pi(void) {
  static char first_execution = 1;
  static float Vmod0 = 0;
  int i;
  float Vref = 220;
  float v_alpha, v_beta, v_0, dc;
  float V_pll;
  const int n_calib = 2048;

  HIGH(SIGCONTROL);
  /* get the previous values into ad_values buffer */
  for (i = 0; i < 8; i++)
    ad_values[i] = ad_data[i] + ad_data[i + 8];
  /* ad_values now holds the values for channel 1, 3, 7, 8, 11, 12, 14 and 15 respectively */
  /* in the case of the first conversion, it will holds 0 */

  HIGH(SIGAD);
  /* start AD reading sequence */
  ADC1->CR2 |= ADC_CR2_SWSTART;

  /* initializes structures in the first time execution */
  if (first_execution) {
    first_execution = 0;
  }
  else {
    /* suspicious piece of code */
    if (tim1_irq_count == 0)
      tim1_irq_count = n_calib + 1;
  }

  /* if machine is turned off, lets find the current offsets
     for ia and ib: ia_offset, ib_offset */
  if (tim1_irq_count > 0 && tim1_irq_count <= n_calib) {
    /* AD values were already read by previous cycle */
    offset_st1 += ad_values[0];
    offset_st2 += ad_values[1];
    offset_st3 += ad_values[2];
    offset_st4 += ad_values[3];
    offset_sc1 += ad_values[4];
    offset_sc2 += ad_values[5];
    offset_sc3 += ad_values[6];
    offset_sc4 += ad_values[7];
    if (tim1_irq_count == n_calib) {
      offset_st1 /= n_calib;
      offset_st2 /= n_calib;
      offset_st3 /= n_calib;
      offset_st4 /= n_calib;
      offset_sc1 /= n_calib;
      offset_sc2 /= n_calib;
      offset_sc3 /= n_calib;
      offset_sc4 /= n_calib;
    }
  }
  else {
    /* Filter */
    { const float fi = 0.0005;
      offset_st1 = fi * ad_values[0] + (1.0 - fi) * offset_st1;
      offset_st2 = fi * ad_values[1] + (1.0 - fi) * offset_st2;
    }

    /* Convert Vab, Vbc and Vca */
    Vab = fest1 * (ad_values[0] - offset_st1);
    Vbc = fest2 * (ad_values[1] - offset_st2);

    /* current for phase a in SC3 */
    ia = fesc3 * (ad_values[6] - offset_sc3);

    /* To test Pll */
    theta2 += omega2 * Tpwm;
    while (theta2 > M_2PI) theta2 -= M_2PI;

    /* Vab = M_sqrt2 * 220 * sin_lookup(theta2); */
    /* Vbc = M_sqrt2 * 220 * sin_lookup(theta2 - A120); */

    Vca = -Vbc - Vab;

    /* TODO: evaluate the modulus of generator voltage by va, vb and vc */
    abc_to_alphabeta(Vab, Vbc, Vca, &v_alpha, &v_beta, &v_0);

    PLL(Vab, Vbc, Vca, &theta, &omega, &Vq);
    /* if (PLL(Vab, Vbc, Vca, &theta, &omega, &Vq)); */
    /*   HIGH(SIGCONTROL); */
    /* else */
    /*   LOW(SIGCONTROL); */

    {
      float fi = 0.5;
      Vmod = M_sqrt_1_3 * sqrtf(sqr(v_alpha) + sqr(v_beta));
      Vmod = fi * Vmod + (1 - fi) * Vmod0;
      Vmod0 = Vmod;
    }

    V_pll = M_sqrt2 * 220 * sin_lookup(theta);

    /* DAs for debug */
    /* set_DA1 ((v_beta / (2 * 250 * M_sqrt3) + 0.5) * (1 << 12) - 1); */
    /* set_DA2 ((omega / 433) * (1 << 12) - 1); */
    /* set_DA1 ((Vab / (2 * M_sqrt2 * 250) + 0.5) * (1 << 12) - 1); */
    /* set_DA2 ((Vbc / (2 * M_sqrt2 * 250) + 0.5) * (1 << 12) - 1); */

    set_DA1(/* st1 */ ad_values[0] >> 1);
    set_DA2((V_pll / (2 * M_sqrt2 * 250) + 0.5) * (1 << 12) - 1);

    /* set_DA2(/\* st2 *\/ ad_values[1] >> 1); */

    /* opmode is set by user button */
    if (opmode == 1) {
      HIGH(RELAY);
      /* opmode == 1 => system is under operation */

      /* Controller comparing V to Vref and chopper current as output */
      Iref = controller_eval(&v_cont, Vref, Vmod);

      /* Controller comparing the actual chopper output current to the one from previous controller */
      dc = controller_eval(&i_cont, Iref, ia);

      /* Apply duty cycle from previous controller in channel 1 bridge leg */
      /* PWM in phase A, duty cycle dc is float from 0 to 1 */
      TIM1->CCR1 = pwm_period * dc;
      leg1_low_switch_PWM();
      /* Bridge leg 2 low switch allways ON */
      TIM1->CCR2 = 0;
      leg2_low_switch_PWM();
      /* Bridge leg 3 HiZ */
      TIM1->CCR3 = 0;
      leg3_low_switch_OFF();
    }
    else {
      /* bridge is off */
      put_legs_high_Z();
      LOW(RELAY);
    }
  }

  LOW(SIGCONTROL);
}
