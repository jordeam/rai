#include <stdint.h>
#include <stdio.h>
#include "gpio_mod.h"
#include "misc.h"
#include "stm32f4xx.h"

#include "circbuf1.h"
#include "pins.h"
#include "uart_mod.h"

#define UARTBUFSIZ 128
#define RXBUFSIZ 20
#define STACKSIZ 2

/* this is the UART circular buffer for transmission */
static circbuf1_t uart_txcbuf, uart_rxcb;
static uint8_t txbuf[UARTBUFSIZ], rxbuf[UARTBUFSIZ];

void uart_init(unsigned int u32_baudUART_bps, void (*rx_hook)(void)) {

  /*enable the USART3 peripheral clock */
  RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

  /* setting the USART_CR1 register*/
  USART3->CR1 |= USART_CR1_UE;     /*USART Enable */
  USART3->CR1 |= /* receiver enable*/ USART_CR1_RE | /*Transmitter Enable*/ USART_CR1_TE;
  /* USART3->CR1 |= USART_CR1_OVER8;  /\*USART Oversmapling 8-bits *\/ */

  /* USART3->CR3 |= USART_CR3_ONEBIT ;  /\*One Bit method*\/ */

  /* USART pins */
  /* USART3 TX232 - PD8 */
  GPIO_config_af(TX232, 7);
  GPIO_config_mode(TX232,GPIO_MODE_AF);
  GPIO_config_out_type(TX232,GPIO_OUT_TYPE_PP);
  GPIO_config_pupd(TX232,GPIO_PORT_PUPD_PU);
  GPIO_config_out_speed(TX232,GPIO_SPEED_HIGH);
  
  /* USART3 RX232 - PD9 */
  GPIO_config_af(RX232, 7);
  GPIO_config_mode(RX232,GPIO_MODE_IN);
  GPIO_config_pupd(RX232,GPIO_PORT_PUPD_NO);

  float r64_uart_div, r64_uart_fraction;
  unsigned int u32_mantissa,u32_fraction;
 
  r64_uart_div = (float) 48000000 / ( u32_baudUART_bps * 16 );
  u32_mantissa = (int) (r64_uart_div);
  r64_uart_fraction =  r64_uart_div - (unsigned int)(r64_uart_div);
  r64_uart_fraction =  r64_uart_fraction * 8;
  u32_fraction =  (int)(r64_uart_fraction);
  if ( u32_fraction >=16 )
    u32_mantissa++;
   
  u32_mantissa = u32_mantissa << 4;
  u32_fraction = u32_fraction & 0x07;

  USART3->BRR = u32_mantissa | u32_fraction;

  /* USART Enable */
  USART3->CR1 |= USART_CR1_UE;

  /* init the circular output buffer */
  circbuf1_init(&uart_txcbuf, txbuf, UARTBUFSIZ, NULL);

  /* init the circular input buffer */
  circbuf1_init(&uart_rxcb, rxbuf, UARTBUFSIZ, rx_hook);

  /* Enable receiver interrupt */
  USART3->CR1 |= USART_CR1_RXNEIE;     

  /* Configure NVIC for USART3 */
  NVIC_SetPriority(USART3_IRQn, 0);
  NVIC_EnableIRQ(USART3_IRQn);
}

int uart_getc(void) {
  return circbuf1_get(&uart_rxcb);
}

/* can not be called by an ISR */
void uart_putc(int c) {
  /* determine if it is called inside USART3 ISR */
  int i = NVIC_GetActive(USART3_IRQn);
  if (i == 0)
    NVIC_DisableIRQ(USART3_IRQn);
  circbuf1_put(&uart_txcbuf, c);
  if (i == 0)
    NVIC_EnableIRQ(USART3_IRQn);
  /* Enable transmiter interrupt */
  USART3->CR1 |= USART_CR1_TXEIE;     
}

/* can not be called by an ISR */
/* Queue put a null terminated string */
void uart_puts(const char *s) {
  /* determine if it is called inside USART3 ISR */
  int i = NVIC_GetActive(USART3_IRQn);
  if (!i)
    NVIC_DisableIRQ(USART3_IRQn);
  while (*s) { 
    circbuf1_put(&uart_txcbuf, *s++);
  }
  if (!i)
    NVIC_EnableIRQ(USART3_IRQn);
  /* Enable transmiter interrupt */
  USART3->CR1 |= USART_CR1_TXEIE;
}

void USART3_IRQHandler(void) {
  int data;
  /* verify if UART Transmit buffer is empty, because this ISR can be also called by end of transmission  */
  if (USART3->SR & USART_SR_TXE) {
    /* get the byte from circular buffer */
    data = circbuf1_get(&uart_txcbuf);
    /* put it to UART, if buffer was not empty */
    if (data > 0)
      USART3->DR = data;
    else
      /* buffer is empty, stop TXE ISR */
      USART3->CR1 &= ~USART_CR1_TXEIE;
  }
  if (USART3->SR & USART_SR_RXNE) {
    data = USART3->DR;
    circbuf1_put(&uart_rxcb, data);
  }
  if (USART3->SR & USART_SR_ORE) {
    data = USART3->DR;
    circbuf1_put(&uart_rxcb, data);
  }
  if (USART3->SR & USART_SR_NE) {
    data = USART3->DR;
    uart_putc('#');
    uart_putc(data);
    uart_puts(" NE\r");
  }
  if (USART3->SR & USART_SR_FE) {
    data = USART3->DR;
    uart_putc('#');
    uart_putc(data);
    uart_puts(" FE\r");
  }
  if (USART3->SR & USART_SR_PE) {
    data = USART3->DR;
    uart_putc('#');
    uart_putc(data);
    uart_puts(" PE\r");
  }
}
