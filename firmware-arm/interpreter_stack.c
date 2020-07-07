#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "stm32f4xx.h"

#include "ad_mod.h"
#include "circbuf1.h"
#include "uart_mod.h"
#include "pins.h"
#include "spi_mod.h"
#include "interpreter_stack.h"

#define RXBUFSIZ 20

/* the stack of data */
int stack[STACKSIZ];
int stacksiz = 0;
/* the received command */
char rec_data = 0;
static char rxbuf[RXBUFSIZ];
int rxi = 0;

/* called by ISR */
void rx_hook(void) {
  rec_data = 1;
}

void stack_put(int data) {
  int i;
  if (stacksiz < STACKSIZ)
    stacksiz++;
  /* we need to shift down stack */
  for (i = stacksiz - 1; i > 0; i--)
    stack[i] = stack[i - 1];
  stack[0] = data;
}

int stack_drop(void) {
  int i, r = stack[0];
  if (stacksiz) {
    for (i = 0; i < stacksiz - 1; i++)
      stack[i] = stack[i + 1];
    stacksiz--;
  }
  return r;
}

/* this is a task */
void interpret_rx(void) {
  char  *endptr;
  int c, val;
  while ((c = uart_getc()) > 0) {
    if (c == ' ' || c == '\n' || c == '\r') {
      /* verify if it is a valid command */
      if (rxi > 0 && rxi < RXBUFSIZ) {
        /* verify if it is a number */
        if ((rxbuf[0] == '-' && rxi > 1) || isdigit((int) rxbuf[0])) {
          /* verify stack overflow */
          rxbuf[rxi] = '\0';
          val = strtol(rxbuf, &endptr, 0);
          /* verify if it is a valid number */
          if (endptr != rxbuf && *endptr == '\0') {
            stack_put(val);
          }
        }
        /* it is a command */
        else {
          rxbuf[rxi] = '\0';
          interpreter_s(rxbuf);
        }
      }
      /* resets rxbuf */
      rxi = 0;
    }
    else {
      if (rxi < RXBUFSIZ) {
        rxbuf[rxi++] = c;
      }
    }
  }
}

void interpreter_s(const char * cmd) {
  int tmp;
  int i;
  char s[20];
  if (strcmp(cmd, "inc") == 0)
    stack[0]++;
  else if (strcmp(cmd, "dup") == 0)
    stack_put(stack[0]);
  else if (strcmp(cmd, "drop") == 0)
    stack_drop();
  else if (strcmp(cmd, "swap") == 0) {
    tmp = stack[0] ;
    stack[0] = stack[1];
    stack[1] = tmp;
  }
  else if (strcmp(cmd, "dac") == 0) {
    /* <value> <dac_num=1 or 2> dac */
    if (stack[0] == 1)
      DAC->DHR12R1 = stack[1];
    else if (stack[0] == 2)
      DAC->DHR12R2 = stack[1];
  }
  else if (strcmp(cmd, "clk") == 0) {
    /* put Processor clock in MHz in stack */
    stack_put(SystemCoreClock / 1000000);
  }
  else if (strcmp(cmd, "stack") == 0) {
    /* printf all stack values without modifying it */
    for (i = 0; i < stacksiz; i++) {
      snprintf(s, 20, "%d ", stack[i]);
      uart_puts(s);
    }
    uart_putc('\r');
  }
  else
    uart_puts("#IC\r");
}
