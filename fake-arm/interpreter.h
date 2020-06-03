#ifndef _interpreter_h
#define _interpreter_h

#include <stddef.h>
#include <stdlib.h>

enum command_access { /* only execute function */ EXEO, /* read_only */ RO, /* write_only */ WO, /* read_n_write */ RW};
enum command_type { none, pstring, string, uint8, uint16, uint32, int8, int16, int32 };
enum command_module { global, vent, infpump, cardmon, dissO2 };

struct command_table {
  char * name;
  enum command_access access;
  enum command_type type;
  size_t offset;
  int (*pre_read)(char *cmd);
  int (*post_write)(char *cmd, char *data);
  char *info;
};

typedef struct command_table command_table_t;

int number_tokens(const char * s, int len);
char * strtrim2(char *s);
int split_tokens(char *s);
char * get_token(char * s, int len, int n);
void interpreter(char * s, char * sendBuff, int sendBuff_size, const void * baseaddr_params[],  const command_table_t * cmdtab);

#endif
