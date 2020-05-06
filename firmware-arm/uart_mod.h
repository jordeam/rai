#ifndef _uart_mod_h
#define _uart_mod_h

#include <stdint.h>

#define UART_BUFSIZE 256

void uart_init(unsigned int u32_baudUART_bps, void (*rx_hook)(void));
void uart_putc(int c);
void uart_puts(const char *s);
int uart_getc(void);

#endif

