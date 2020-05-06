#include "spi_mod.h"
#include "pbit.h"
#include "pins.h"
#include "gpio_mod.h"

// communication structures and vector
uint16_t spi_txdata[4] = { 0xffaa, 0xffbb, 0xffcc, 0xffdd };
uint16_t spi_rxdata[4] = { 0xffaa, 0xffbb, 0xffcc, 0xffdd }; 

/* pending IRQ6 (OINT from slave) to start SPI */
int n_oints = 10;

static void (*control_fcn)(void);

/* SPI3 */
void spi_init(void (*fcn)(void)) {
  control_fcn = fcn;

  /* /\* Enable SPI3 clock *\/ */
  RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;

  /*** Configure pins ***/
  /* Configure NSS for alternate function */
  GPIO_config_mode(NSS, GPIO_MODE_AF);
  GPIO_config_out_type(NSS, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(NSS, GPIO_SPEED_HIGH);
  GPIO_config_pupd(NSS, GPIO_PORT_PUPD_PU);
  GPIO_config_af(NSS, /* SPI3 */ 6);

  /* Configure SCK for alternate function */
  GPIO_config_mode(SCK, GPIO_MODE_AF);
  GPIO_config_out_type(SCK, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(SCK, GPIO_SPEED_HIGH);
  GPIO_config_pupd(SCK, GPIO_PORT_PUPD_NO);
  GPIO_config_af(SCK, /* SPI3 */ 6);

  /* Configure MOSI for alternate function */
  GPIO_config_mode(MOSI, GPIO_MODE_AF);
  GPIO_config_out_type(MOSI, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(MOSI, GPIO_SPEED_HIGH);
  GPIO_config_pupd(MOSI,GPIO_PORT_PUPD_NO);
  GPIO_config_af(MOSI, /* SPI3 */ 6);

  /* Configure MISO */
  GPIO_config_mode(MISO, GPIO_MODE_AF);
  GPIO_config_out_type(MISO, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(MISO, GPIO_SPEED_HIGH);
  GPIO_config_pupd(MISO, GPIO_PORT_PUPD_NO);
  GPIO_config_af(MISO, /* SPI3 */ 6);

  /* SPI3 Configuration */
  SPI3->CR1 = SPI_CR1_DFF | (/* BR */ 3 << 3) | SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA;
  SPI3->CR2 |= SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN | SPI_CR2_SSOE;

  /* /\* Enable DMA clock *\/ */
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

  /* Configure DMA stream 5 channel 0 for SPI3_TX */
  DMA1_Stream5->CR = (/*CHSEL*/ 0 << 25)| (/*PL*/ 2 << 16) | (/*MSIZE*/ 1 << 13) | (/*PSIZEW*/ 1 << 11) | DMA_SxCR_MINC | DMA_SxCR_TCIE | (/*DIR*/ 1 << 6);
  DMA1_Stream5->NDTR = 4;
  DMA1_Stream5->PAR = (uint32_t) &SPI3->DR;
  DMA1_Stream5->M0AR = (uint32_t) spi_txdata;

  /* Configure DMA stream 2 channel 0 for SPI3_RX */
  DMA1_Stream2->CR = (/*CHSEL*/ 0 << 25)| (/*PL*/ 2 << 16) | (/*MSIZE*/ 1 << 13) | (/*PSIZEW*/ 1 << 11) | DMA_SxCR_MINC | DMA_SxCR_TCIE | (/*DIR*/ 0 << 6);
  DMA1_Stream2->NDTR = 4;
  DMA1_Stream2->PAR = (uint32_t) &SPI3->DR;
  DMA1_Stream2->M0AR = (uint32_t) spi_rxdata;

  /* Configure NVIC for DMA1 Streams 2 and 5 and SPI3 */
  NVIC_SetPriority(DMA1_Stream5_IRQn, 2);
  NVIC_EnableIRQ(DMA1_Stream5_IRQn);

  NVIC_SetPriority(DMA1_Stream2_IRQn, 2);
  NVIC_EnableIRQ(DMA1_Stream2_IRQn);

  NVIC_SetPriority(SPI3_IRQn, 2);
  NVIC_EnableIRQ(SPI3_IRQn);
}

/*
  EXTI IRQ3
  This interrupt is triggered by an external signal, coming from SLAVE.
  It will count n_oints pulses before starting SPI
*/
void EXTI3_IRQHandler(void) {
  /* verify if external IRQ line 3 is pending */
  if (EXTI->PR & EXTI_PR_PR3) {
    /* start SPI to transmit duty cycles */
    if (n_oints)
      n_oints--;
    else {
      HIGH(SIGI);
      spi_start();
    }
    /* clear pending bit writing 1 to bit 6 */
    EXTI->PR |= EXTI_PR_PR3;
  }
}

/* 
   spi_start is called after a rising edge of pin 6 of PORTC and it initiates SPI transmission, as master
*/
void spi_start(void) {
  uint32_t res = 0; 
  while(SPI3->SR & SPI_SR_BSY);
  if (SPI3->SR & SPI_SR_RXNE)
    res = SPI3->DR;
  res=res;
  /* reload NDTR and M0AR*/
  DMA1_Stream5->NDTR = 4;
  DMA1_Stream5->M0AR = (uint32_t) spi_txdata;
  DMA1_Stream2->NDTR = 4;
  DMA1_Stream2->M0AR = (uint32_t) spi_rxdata;
  /* enable DMA stream 2 and 5 */
  DMA1_Stream5->CR |= DMA_SxCR_EN;
  DMA1_Stream2->CR |= DMA_SxCR_EN;
  /* enable SPI to initiate transmission */
  SPI3->CR1 |= SPI_CR1_SPE;
}

/* SPI IRQ */
void SPI3_IRQHandler(void) {
   while(SPI3->SR & SPI_SR_BSY);
   /* disable SPI */
   SPI3->CR1 &= ~SPI_CR1_SPE;
   /* disable SPI TXE interrupt */
   SPI3->CR2 &= ~SPI_CR2_TXEIE;
}

/* DMA SPI TX IRQ */
void DMA1_Stream5_IRQHandler(void) {
  if (DMA1->HISR & DMA_HISR_TCIF5) {
    /* disable DMA stream 4 */
    DMA1_Stream5->CR &= ~DMA_SxCR_EN;
    /* clear flag */
    DMA1->HIFCR |= DMA_HIFCR_CTCIF5;
  }
}

/* DMA SPI RX IRQ */
void DMA1_Stream2_IRQHandler(void) {
  /* wait if SPI is busy */
  while(SPI3->SR & SPI_SR_BSY);
  /* disable SPI */
  SPI3->CR1 &= ~SPI_CR1_SPE;
  if (DMA1->LISR & DMA_LISR_TCIF2) {
    /* disable DMA stream 2 */
    DMA1_Stream2->CR &= ~DMA_SxCR_EN;
    /* clear flag */
    DMA1->LIFCR |= DMA_LIFCR_CTCIF2;
  }
  /* flag using external PIN */
  LOW(SIGI);
}
