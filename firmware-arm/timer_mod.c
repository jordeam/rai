#include <stdlib.h>
#include <stdint.h>
#include "stm32f4xx.h"

#include "pins.h"
#include "gpio_mod.h"
#include "timer_mod.h"
#include "spi_mod.h"

static void (*timer1_fcn)(void) = NULL;
static void (*timer3_fcn)(void) = NULL;

uint16_t pwm_period = 1200;

/* counts how many times timer 1 IRQ occurred */
int tim1_irq_count = 0;

void timer1_set_fcn(void (*fcn)(void)) {
  timer1_fcn = fcn;
}

void timer3_set_fcn(void (*fcn)(void)) {
  timer3_fcn = fcn;
}

/* PWM */
void TIM1_init(void) {
  uint32_t reg;
  
  /* pins were configured at pins.c */

  /* Configure TIMER 1 for PWM operation */
    /* TIM1 module clock */
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

  /* TIMx_CR1 */
  /* clock div = 0, edge, up, update */
  reg = TIM1->CR1 & 0xfc00;
  reg |= TIM_CR1_ARPE | TIM_CR1_URS;
  TIM1->CR1 = reg;

  TIM1->ARR = pwm_period;
  TIM1->PSC = /* divide 24MHz by 14 */ 13;

  /* TRGO source is from update and deadtime for CH1 and CH1N */
  /* TIMx_CR2 */
  reg = TIM1->CR2 & (BIT15 | BIT1);
  reg &= ~TIM_CR2_MMS;
  reg |= TIM_CR2_MMS_1 /* | TIM_CR2_OIS1N | TIM_CR2_OIS1 */;
  TIM1->CR2 = reg;

  /* PWM Mode 1 */
  /* TIMx_CCMR1 */
  reg = TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE;
  reg |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;
  TIM1->CCMR1 = reg;

  /* TIMx_CCMR2 */
  reg = TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4PE;
  reg |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3PE;
  TIM1->CCMR2 = reg;

  /* TIMx_CCER */
  reg = TIM1->CCER & (BIT15 | BIT14);
  reg = TIM_CCER_CC1E | TIM_CCER_CC1NE;
  reg |= TIM_CCER_CC2E | TIM_CCER_CC2NE;
  reg |= TIM_CCER_CC3E | TIM_CCER_CC3NE;
  reg |= TIM_CCER_CC4E;
  TIM1->CCER = reg;

  /* duty cycle */
  TIM1->CCR1 = 0;
  TIM1->CCR2 = 0;
  TIM1->CCR3 = 0;
  TIM1->CCR4 = /* width */ 100;

  /* TIMx_BDTR Enable the TIM Main Output */
  TIM1->BDTR = TIM_BDTR_MOE | /* Dead Time Generation */ 10;

  /* TIMx_DIER */
  reg = TIM1->DIER & BIT15;
  reg |= TIM_DIER_UIE;
  TIM1->DIER = reg;
 
  NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 4);
  NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

  /* enable TIM1 counter */
  TIM1->CR1 |= TIM_CR1_CEN;

  /* Sets high side to PWM mode, once they have 0 duty cycle,
     its switches will be off */
  GPIO_config_mode(PWM1, GPIO_MODE_AF);
  GPIO_config_out_type(PWM1, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(PWM1, GPIO_SPEED_HIGH);
  GPIO_config_pupd(PWM1,GPIO_PORT_PUPD_PU);
  GPIO_config_af(PWM1, 1);

  GPIO_config_mode(PWM2, GPIO_MODE_AF);
  GPIO_config_out_type(PWM2, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(PWM2, GPIO_SPEED_HIGH);
  GPIO_config_pupd(PWM2,GPIO_PORT_PUPD_PU);
  GPIO_config_af(PWM2, 1);

  GPIO_config_mode(PWM3, GPIO_MODE_AF);
  GPIO_config_out_type(PWM3, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(PWM3, GPIO_SPEED_HIGH);
  GPIO_config_pupd(PWM3,GPIO_PORT_PUPD_PU);
  GPIO_config_af(PWM3, 1);

  /* Release ENALL */
  LOW(ENALL);
}

/* Encoder input via TIMER 4 */
void TIM4_init(void) {
 uint32_t reg;

  /* GPIO_config */
  /* TIM4 CH1 as alternate function push-pull -- ENC1A - PB4*/
  GPIO_config_mode(ENC1A, GPIO_MODE_AF);
  GPIO_config_out_type(ENC1A, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(ENC1A, GPIO_SPEED_HIGH);
  GPIO_config_pupd(ENC1A,GPIO_PORT_PUPD_PU);
  GPIO_config_af(ENC1A, 2);

  /* TIM4 CH2 as alternate function push-pull -- ENC2A - PB5*/
  GPIO_config_mode(ENC1B, GPIO_MODE_AF);
  GPIO_config_out_type(ENC1B, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(ENC1B, GPIO_SPEED_HIGH);
  GPIO_config_pupd(ENC1B,GPIO_PORT_PUPD_PU);
  GPIO_config_af(ENC1B, 2);

  /* TIM4 module clock */
  RCC->APB1ENR|= RCC_APB1ENR_TIM4EN;

  /* TIMx_CR1 */
  /* clock div = 0, edge, up, update */
  reg = TIM4->CR1 & 0xfc00;
  reg |= TIM_CR1_ARPE | TIM_CR1_URS;
  TIM4->CR1 = reg;

  /* TRGO source is from update */
  /* TIMx_CR2 */
  reg = TIM4->CR2 & 0xff07;
  reg &= ~TIM_CR2_MMS;
  reg |= TIM_CR2_MMS_1;
  TIM4->CR2 = reg;

  /* TIMx_SlaveModeControlRegister SMCR*/
  TIM4->SMCR |= (3/*SMS=011*/);

  /* TIMx_DIER */
  TIM4->DIER |= TIM_DIER_UIE;

  /* TIMx_CCMR1 */
  reg = TIM4->CCMR1;
  reg |= TIM_CCMR1_CC1S_0| TIM_CCMR1_CC2S_0; 
  TIM4->CCMR1 =reg;

  /* TIMx_CCER */
  TIM4->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E ;

  TIM4->ARR = 4004;
  TIM4->PSC = 0;  

  /* TIMx_CCER */
  NVIC_SetPriority(TIM4_IRQn, 4);
  NVIC_EnableIRQ(TIM4_IRQn);

  /* enable TIM4 counter */
  TIM4->CR1 |= TIM_CR1_CEN;
}

/* speed control via TIM3 */
void TIM3_init(void) {
 uint32_t reg;

  /* TIM3 module clock */
  RCC->APB1ENR|= RCC_APB1ENR_TIM3EN;

  /* TIMx_CR1 */
  /* clock div = 0, edge, up, update */
  reg = TIM3->CR1 & 0xfc00;
  reg |= TIM_CR1_ARPE | TIM_CR1_URS;
  TIM3->CR1 = reg;

  /* TRGO source is from update */
  /* TIMx_CR2 */
  reg = TIM3->CR2 & 0xff07;
  reg &= ~TIM_CR2_MMS;
  reg |= TIM_CR2_MMS_1;
  TIM3->CR2 = reg;

  /* TIMx_SlaveModeControlRegister SMCR*/
  TIM3->SMCR |= (0/*SMS=000*/);

  /* TIMx_DIER */
  TIM3->DIER |= TIM_DIER_UIE;

  /* TIMx_CCMR1 */
  reg = TIM3->CCMR1;
  reg &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_CC1S);
  reg |= TIM_CCMR1_OC1M_0;
  TIM3->CCMR1 =reg;

  /* TIMx_CCER */
  TIM3->CCER |= TIM_CCER_CC1E;

  TIM3->ARR = 500;   //320Hz
  TIM3->PSC = (uint16_t) ((SystemCoreClock / 2) / 500000) - 1;      
  TIM3->CCR1= 512;

  /* TIMx_CCER */
  NVIC_SetPriority(TIM3_IRQn, 4);
  NVIC_EnableIRQ(TIM3_IRQn);

  /* enable TIM3 counter */
  TIM3->CR1 |= TIM_CR1_CEN;
}

void TIM4_IRQHandler(void) {
  volatile uint16_t encoder = 0;
  encoder++; /* initializing and incrementing just to shut up compilation warning */
  encoder = TIM4->SR;
  /* clear update event flag */
  TIM4->SR &= ~(TIM_SR_UIF) ;
  encoder = TIM4->SR;
}

void TIM3_IRQHandler(void) {
  /* clear update event flag */
  TIM3->SR &= ~(TIM_SR_UIF) ;
  if (timer3_fcn)
    timer3_fcn();
}

void TIM1_UP_TIM10_IRQHandler(void) {
  //  HIGH(SIGCONTROL); /* visual pin flag */
  /* clear update event flag */
  TIM1->SR &= ~TIM_SR_UIF;
  if (timer1_fcn) {
    timer1_fcn();
  }
  tim1_irq_count++;
  //  LOW(SIGCONTROL); /* visual flag */
}
