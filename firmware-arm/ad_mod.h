#ifndef _ad_mod_h
#define _ad_mod_h

#include <sys/types.h>

#define ADBUFSIZ 16

extern uint16_t ad_data[ADBUFSIZ];

void AD_init(void);
u16 readAD1(void);

void DA_init(void);

#define set_DA1(x) DAC->DHR12R1 = x
#define set_DA2(x) DAC->DHR12R2 = x

#endif

