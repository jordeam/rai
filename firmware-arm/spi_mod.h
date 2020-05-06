#ifndef _spi_mod_h
#define _spi_mod_h
#include <stdint.h>

extern uint16_t spi_txdata[4];
extern uint16_t spi_rxdata[4];

void spi_init(void (*fcn)(void));
void spi_start(void);

#endif
