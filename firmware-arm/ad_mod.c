#include <sys/types.h>

#include "stm32f4xx.h"

#include "pbit.h"
#include "pins.h"
#include "gpio_mod.h"
#include "stasks_mod.h"
#include "spi_mod.h"

#include "ad_mod.h"

/* currents read from injected mode */
int j_i0, j_i1, j_i2, j_i3, j_ia = 0, j_ib = 0;

uint16_t ad_data[ADBUFSIZ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void AD_init(void) {
  uint32_t reg;

    /* AD INPUTS */
  GPIO_config_mode(ADC_IN1, GPIO_MODE_ANALOG);
  GPIO_config_pupd(ADC_IN1, GPIO_PORT_PUPD_NO);
  GPIO_config_mode(ADC_IN3, GPIO_MODE_ANALOG);
  GPIO_config_pupd(ADC_IN3, GPIO_PORT_PUPD_NO);
  GPIO_config_mode(ADC_IN7, GPIO_MODE_ANALOG);
  GPIO_config_pupd(ADC_IN7, GPIO_PORT_PUPD_NO);
  GPIO_config_mode(ADC_IN8, GPIO_MODE_ANALOG);
  GPIO_config_pupd(ADC_IN8, GPIO_PORT_PUPD_NO);
  GPIO_config_mode(ADC_IN11, GPIO_MODE_ANALOG);
  GPIO_config_pupd(ADC_IN11, GPIO_PORT_PUPD_NO);
  GPIO_config_mode(ADC_IN12, GPIO_MODE_ANALOG);
  GPIO_config_pupd(ADC_IN12, GPIO_PORT_PUPD_NO);
  GPIO_config_mode(ADC_IN14, GPIO_MODE_ANALOG);
  GPIO_config_pupd(ADC_IN14, GPIO_PORT_PUPD_NO);
  GPIO_config_mode(ADC_IN15, GPIO_MODE_ANALOG);
  GPIO_config_pupd(ADC_IN15, GPIO_PORT_PUPD_NO);


  /* It is not necessary to alter RCC_CFGR */
  /* reg = RCC->CFGR; */
  /* RCC->CFGR = reg; */
  
  /* Enable ADC1 clock so that we can program it */
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

  /* pre CR1 configuration */
  ADC1->CR1 &= 0xf83f0000;

  reg = ADC1->CR2 & 0x8080f0fc;
  /*some bits to be set; */
  /* Enable ADC1 */
  reg |= ADC_CR2_ADON; 
  ADC1->CR2 = reg;

  /* configure samples of each channel: channels 1 3 7 8 11 12 14 15 */
  reg = ADC1->SMPR1 & 0xf0000000;
  reg |= /* 15 cycles */ ADC_SMPR1_SMP15_1 | /* 15 cycles */ ADC_SMPR1_SMP14_1 | /* 15 cycles */ ADC_SMPR1_SMP12_1 | /* 15 cycles */ ADC_SMPR1_SMP11_1;
  ADC1->SMPR1 = reg;
  reg = ADC1->SMPR2 & 0xc0000000;
  reg |= /* 15 cycles */ ADC_SMPR2_SMP8_1 | /* 15 cycles */ ADC_SMPR2_SMP7_1 | /* 15 cycles */ ADC_SMPR2_SMP3_1 | /* 15 cycles */ ADC_SMPR2_SMP1_1;
  ADC1->SMPR2 = reg;

  /* offset */
  ADC1->JOFR1 = ADC1->JOFR1 & ~((uint32_t) ADC_JOFR1_JOFFSET1);
  ADC1->JOFR2 = ADC1->JOFR2 & ~((uint32_t) ADC_JOFR2_JOFFSET2);
  ADC1->JOFR3 = ADC1->JOFR3 & ~((uint32_t) ADC_JOFR3_JOFFSET3);
  ADC1->JOFR4 = ADC1->JOFR4 & ~((uint32_t) ADC_JOFR4_JOFFSET4);

  /* injected channel sequence */
  reg = ADC1->JSQR & 0xffc00000;
  /* 4 conversions: */ 
  reg |= ADC_JSQR_JL_1 | ADC_JSQR_JL_0;
  /* channels 1 3 1 3 */
  reg |= (/* ch */ 1 << (5 * 3)) | (/* ch */ 3 << (5 * 2)) | (/*ch 1 */ 1 << 5) | /* ch */ 3;
  ADC1->JSQR = reg;

  /* Enable AD injected channel interrupt and scan mode */
  reg = ADC1->CR1 & 0xf83f0000;
  reg |= ADC_CR1_SCAN | ADC_CR1_EOCIE | ADC_CR1_JEOCIE;
  ADC1->CR1 = reg;

  /* Configure NVIC for AD1 */
  NVIC_SetPriority(ADC_IRQn, 3);
  NVIC_EnableIRQ(ADC_IRQn);

  /* Regular Sequence Channels */
  reg = ADC1->SQR1 & 0xff000000;
  reg |= /* L length */ 15 << 20;
  reg |= /* SQ16 */ 15 << 15 | /* SQ15 */ 14 << 10 | /* SQ14 */ 12 << 5 | /* SQ13 */ 11;
  ADC1->SQR1 = reg;
  reg = ADC1->SQR2 & 0xc0000000;
  reg |= /* SQ12 */ 8 << 25 | /* SQ11 */ 7 << 20 | /* SQ10 */ 3 << 15 | /* SQ9 */ 1 << 10;
  reg |= /* SQ8 */ 15 << 5 | /* SQ7 */ 14;
  ADC1->SQR2 = reg;
  reg = ADC1->SQR3 & 0xc0000000;
  reg |= /* SQ6 */ 12 << 25 | /* SQ5 */ 11 << 20 | /* SQ4 */ 8 << 15 | /* SQ3 */ 7 << 10;
  reg |= /* SQ2 */ 3 << 5 | /* SQ1 */ 1;
  ADC1->SQR3 = reg;

  /* Configure DMA2 Stream 0 Channel  for ADC1 */
  /* TODO: verify EOCS and EOC, EOCS = 0 (?) */
  ADC1->CR2 |= /* ADC_CR2_EOCS | */ ADC_CR2_DMA | ADC_CR2_DDS;

  /* /\* Enable DMA2 clock *\/ */
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

  /* Configure DMA2 stream 0 channel 0 for ADC1 */
  DMA2_Stream0->CR = (/*CHSEL*/ 0 << 25)| (/*PL*/ 2 << 16) | (/*MSIZE*/ 1 << 13) | (/*PSIZEW*/ 1 << 11) | DMA_SxCR_MINC | DMA_SxCR_TCIE | (/*DIR*/ 0 << 6);
  DMA2_Stream0->NDTR = ADBUFSIZ;
  DMA2_Stream0->PAR = (uint32_t) &ADC1->DR;
  DMA2_Stream0->M0AR = (uint32_t) ad_data;

  /* Configure NVIC for DMA2 Streams 0 */
  NVIC_SetPriority(DMA2_Stream0_IRQn, 2);
  NVIC_EnableIRQ(DMA2_Stream0_IRQn);

  /* enable DMA2 stream 0 */
  DMA2_Stream0->CR |= DMA_SxCR_EN;
}

void DMA2_Stream0_IRQHandler(void) {
  uint32_t reg = DMA2->LISR;
  if (reg & DMA_LISR_TCIF0) {
    /* clear flag */
    DMA2->LIFCR |= DMA_LIFCR_CTCIF0;
  }
  reg = DMA2->LISR;
  if (reg & DMA_LISR_HTIF0) {
    /* clear flag */
    DMA2->LIFCR |= DMA_LIFCR_CHTIF0;
  }
}

/* uint16_t readAD1(void) { */
/*   /\* start conversion *\/ */
/*   ADC1->CR2 |= ADC_CR2_SWSTART | ADC_CR2_EXTTRIG; */
/*   while (ADC1->SR & ADC_SR_EOC); */
/*   return ADC1->DR; */
/* } */

void ADC_IRQHandler(void) {
  static uint16_t data;
  volatile uint32_t status;
  static uint16_t count;
  data++; /* just to shut up compiling warning*/
  status = ADC1->SR;
  if (status & ADC_SR_STRT /* EOC will be cleared by last DMA transfer if EOC is set, DMA is not working */
      /* TODO: verify it it enters interruption with EOC set */
      /* && status & ADC_SR_EOC */) {
    /* Reprogram DMA2 Stream 0 */
    /* disable DMA2 stream 0 */
    DMA2_Stream0->CR &= ~DMA_SxCR_EN;
    count = DMA2_Stream0->NDTR;
    count += 0;
    DMA2_Stream0->NDTR = ADBUFSIZ;
    DMA2_Stream0->M0AR = (uint32_t) ad_data;
    /* clear DMA flags */
    DMA2->LIFCR |= DMA_LIFCR_CTCIF0;
    /* renable DMA2 stream 0 */
    DMA2_Stream0->CR |= DMA_SxCR_EN;
    /* read DR only to clear INT pending bit */
    data = ADC1->DR;
    /* clear software start bit */
    ADC1->SR &= ~ADC_SR_STRT;
  }
  if (status & ADC_SR_JEOC) {
    j_i0 = ADC1->JDR1;
    j_i1 = ADC1->JDR2;
    j_i2 = ADC1->JDR3;
    j_i3 = ADC1->JDR4;
    /* phases a and b current, integer type */
    j_ia = (j_i0 + j_i2) >> 1;
    j_ib = (j_i1 + j_i3) >> 1;
    ADC1->SR &= ~(ADC_SR_JEOC | ADC_SR_JSTRT);
  }
  LOW(SIGAD);
}

void DA_init(void) {
  RCC->APB1ENR |= RCC_APB1ENR_DACEN;
  DAC->CR = (DAC->CR & 0xc000c000) | DAC_CR_TSEL2_2 | DAC_CR_TSEL2_1 | DAC_CR_TSEL2_0 | DAC_CR_TSEL1_2 | DAC_CR_TSEL1_1 | DAC_CR_TSEL1_0;
  DAC->CR |= DAC_CR_EN2 | DAC_CR_EN1;
  DAC->DHR12R1 = 2048; /* (1 << 11); */
  DAC->DHR12R2 = 2048; /* (1 << 11); */
}
