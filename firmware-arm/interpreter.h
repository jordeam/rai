#ifndef _interpreter_h
#define _interpreter_h

#define STACKSIZ 3

void interpret_rx(void);
void interpreter(char cmd);
void interpreter_s(const char * cmd);
void rx_hook(void);

#endif
