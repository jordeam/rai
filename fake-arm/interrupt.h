#ifndef _interrupt_h
#define interrup_h

#include "sttmach.h"

enum control_mode {control_speed, control_position, control_torque};

void phase_reset(sttmach_t *self);
void phase_O2(sttmach_t *self);
void phase_air(sttmach_t *self);
void phase_inspiration(sttmach_t *self);
void phase_forced_expiration(sttmach_t *self);

void interrupt_init(void);

#endif
