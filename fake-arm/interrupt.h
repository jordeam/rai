#ifndef _interrupt_h
#define interrup_h

void * phase_reset(int phase_i);
void * phase_O2(int phase_i);
void * phase_air(int phase_i);
void * phase_inspiration(int phase_i);
void * phase_forced_expiration(int phase_i);

void interrupt_init(void);
void * interval_code(int *max_counter);

#endif
