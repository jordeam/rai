#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdint.h>
#include <stddef.h>

#include "interpreter.h"

/*
  Returns the number of tokens in s separated by nongraphical character.
*/
int number_tokens(const char * s, int len) {
  int i, count = 0, was_graph = 0;
  for (i = 0; i < len; i++)
    if (isgraph(s[i])) {
      if (!was_graph)
        count++;
      was_graph = 1;
    }
    else was_graph = 0;
  return count;
}

/*
  Remove the beginning and ending spaces or non graphical characters.
*/
char * strtrim2(char *s) {
  int i, j, len;
  /* Let's trim the end */
  for (i = strlen(s) - 1; !isgraph(s[i]) && i >= 0; i--)
    s[i] = '\0';
  /* let's trim the beginning */
  len = strlen(s);
  for (i = 0; !isgraph(s[i]) && i < len; i++);
  for (j = 0; i < len; i++, j++)
    s[j] = s[i];
  s[j] = '\0';
  return s;
}

/*
  Remove the beginning and ending spaces ou non graphical characters, and multiple graphical characters between worlds, letting only one non-grafical character, e.g., '\0' so it is easy to get the tokens. Returns string size to be used with get_token and number_token.
*/
int split_tokens(char *s) {
  int i, j, len = strlen(s);
  /* let's trim the beginning */
  for (i = 0; !isgraph(s[i]) && i < len; i++);
  /* Let's split the string */
  for (j = 0; i < len; j++)
    if (isgraph(s[i]))
      s[j] = s[i++];
    else {
      s[j] = '\0';
      while(!isgraph(s[++i]) && i < len);
    }
  s[j] = '\0';
  /* Let's trim the end */
  for (i = j; !isgraph(s[i]) && i >= 0; i--)
    s[i] = '\0';
  return i + 1;
}

/*
  Returns the pointer to nth token.
*/
char * get_token(char * s, int len, int n) {
  int i, count = 0, was_graph = 0;
  if (n == count) return s;
  for (i = 0; i < len; i++)
    if (isgraph(s[i])) {
      if (!was_graph) {
        count++;
        if (n == count - 1)
          return s + i;
      }
      was_graph = 1;
    }
    else was_graph = 0;
  return NULL;
}

/* 
   Interprets a command line in string s.
*/
void interpreter(char * s, char * sendBuff, int sendBuff_size, const void * baseaddr_params[], const command_table_t * cmdtab) {
  int n_tokens, s_len, s_orig_len, n_pump = -1, is_set_cmd;
  int i, cmd_not_found = 1;
  char *data = NULL;
  const void * basepparam = baseaddr_params[0];
  
  s_orig_len = split_tokens(s);
  n_tokens = number_tokens(s, s_orig_len);
  s_len = strlen(s);
  
  /* verifiy if it is an empty command */
  if (s_len == 0) {
    printf("Empty command, doing nothing\n");
    return;
  }
  
  /* verify if is a set command */
  if (s[s_len - 1] == '!') {
    is_set_cmd = 1;
    /* erase '!' char of s */
    s[--s_len] = '\0';
  }
  else
    is_set_cmd = 0;

  /* verify if it is not a global command */
  if (strlen(s) > 3 && s[1] == ':') {
    switch (s[0]) {
    case 'r':
      /* ventilator */
      basepparam = baseaddr_params[vent];
      break;
    case '1':
    case '2':
    case '3':
    case '4':
      /* infusion pumps */
      /* n_pump starts from 0 */
      n_pump = s[0] - '1';
      s[0] = 'i';
      basepparam = baseaddr_params[infpump];
      break;
    case 'm':
      /* cardiac monitor */
      break;
    case 'o':
      /* dissolved O2 */
      break;
    default:
      /* global parameter */
      /* TODO: this module does not exist */
      printf("ERROR: Invalid module:'%c', %s\n", s[0], s);
      sprintf(sendBuff, "%s%s unknown-module\n", s, (is_set_cmd ? "!" : ""));
      return;
    }
  }

  for (i = 0; cmdtab[i].name && cmd_not_found; i++) {
    if (strcmp(cmdtab[i].name, s) == 0) {
      cmd_not_found = 0;
      if (n_pump >= 0)
        /* restore infusion pump number */
        s[0] = '1' + n_pump;
      if (is_set_cmd) {
        if (cmdtab[i].access == RO) {
          /* ERROR: read only parameter */
          printf("ERROR: cmd=%s!, %s: read only parameter\n", s, cmdtab[i].info);
          snprintf(sendBuff, sendBuff_size, "%s! read-only-reg\n", s);
        }
        else if (cmdtab[i].access == EXEO) {
          /* ERROR: exec only command */
          printf("ERROR: cmd=%s!, %s: exec only command\n", s, cmdtab[i].info);
          snprintf(sendBuff, sendBuff_size, "%s! exec-only-cmd\n", s);
        }
        else if (n_tokens != 2) {
          /* ERROR: exec only command */
          printf("ERROR: cmd=%s!, %s: wrong number of parameters\n", s, cmdtab[i].info);
          snprintf(sendBuff, sendBuff_size, "%s! parameter-number-error\n", s);
        }
        else {
          /* No errors */
          data = get_token(s, s_orig_len, 1);
          switch (cmdtab[i].type) {
          case none:
            /* used in exec only commands */
            break;
          case pstring:
            strcpy(*(char **)(basepparam + cmdtab[i].offset), data);
            break;
          case string:
            strcpy((char *)(basepparam + cmdtab[i].offset), data);
            break;
          case uint8:
            *(uint8_t *) (basepparam + cmdtab[i].offset) = atoi(data);
            break;
          case uint16:
            *(uint16_t *) (basepparam + cmdtab[i].offset) = atoi(data);
            break;
          case uint32:
            *(uint32_t *) (basepparam + cmdtab[i].offset) = atoi(data);
            break;
          case int8:
            *(int8_t *) (basepparam + cmdtab[i].offset) = atoi(data);
            break;
          case int16:
            *(int16_t *) (basepparam + cmdtab[i].offset) = atoi(data);
            break;
          case int32:
            *(int32_t *) (basepparam + cmdtab[i].offset) = atoi(data);
            break;
          }
          printf("SET: cmd=%s!, %s\n", s, cmdtab[i].info);
          snprintf(sendBuff, sendBuff_size, "%s! success\n", s);
          if (cmdtab[i].post_write)
            cmdtab[i].post_write(s, data);
        }
      }
      else {
        /* get command */
        if (cmdtab[i].access == WO) {
          /* ERROR: write only parameter */
          printf("ERROR: cmd=%s, %s: write only parameter\n", s, cmdtab[i].info);
          snprintf(sendBuff, sendBuff_size, "%s write-only-reg\n", s);
        }
        else if (cmdtab[i].access == EXEO) {
          /* Exec only command */
          printf("INFO: cmd=%s, %s: executing\n", s, cmdtab[i].info);
          if (n_tokens != 1) {
          /* ERROR: exec only command */
            printf("ERROR: cmd=%s, %s: wrong number of parameters\n", s, cmdtab[i].info);
            snprintf(sendBuff, sendBuff_size, "%s parameter-number-error\n", s);
          }
          else if (cmdtab[i].pre_read) {
            if (cmdtab[i].pre_read(s) == 0) {
              printf("INFO: cmd=%s, %s: executed successfuly\n", s, cmdtab[i].info);
              snprintf(sendBuff, sendBuff_size, "%s success\n", s);
            }
            else {
              printf("ERROR: cmd=%s, %s: error in execution\n", s, cmdtab[i].info);
              snprintf(sendBuff, sendBuff_size, "%s exec-error\n", s);
            }
          }
          else {
            /* Exec function not implemented */
              printf("ERROR: cmd=%s, %s: exec function not implemented\n", s, cmdtab[i].info);
              snprintf(sendBuff, sendBuff_size, "%s exec-error\n", s);
          }
        }
        else if (n_tokens != 1) {
          /* ERROR: exec only command */
          printf("ERROR: cmd=%s, %s: wrong number of parameters\n", s, cmdtab[i].info);
          snprintf(sendBuff, sendBuff_size, "%s parameter-number-error\n", s);
        }
        else {
          int pre_read_status = 0;
          /* No errors */
          if (cmdtab[i].pre_read)
            pre_read_status = cmdtab[i].pre_read(s);
          if (pre_read_status != 0) {
            /* ERROR in execution of pre-read function */
            printf("ERROR: cmd=%s, %s: error in execution of pre_read function\n", s, cmdtab[i].info);
            snprintf(sendBuff, sendBuff_size, "%s exec-error\n", s);
          }
          else {
            if (cmdtab[i].pre_read)
              printf("INFO: cmd=%s, %s: executed successfuly\n", s, cmdtab[i].info);
            switch (cmdtab[i].type) {
            case none:
              /* used for execonly command */
              break;
            case pstring:
              snprintf(sendBuff, sendBuff_size, "%s %s\n", s, *(char **)(basepparam + cmdtab[i].offset));
              break;
            case string:
              snprintf(sendBuff, sendBuff_size, "%s %s\n", s, (char *)(basepparam + cmdtab[i].offset));
              break;
            case uint8:
              snprintf(sendBuff, sendBuff_size, "%s %hhu\n", s, *(uint8_t *)(basepparam + cmdtab[i].offset));
              break;
            case uint16:
              snprintf(sendBuff, sendBuff_size, "%s %hhu\n", s, *(uint16_t *)(basepparam + cmdtab[i].offset));
            break;
            case uint32:
              snprintf(sendBuff, sendBuff_size, "%s %lu\n", s, *(uint32_t *)(basepparam + cmdtab[i].offset));
              break;
            case int8:
              snprintf(sendBuff, sendBuff_size, "%s %hhd\n", s, *(int8_t *)(basepparam + cmdtab[i].offset));
              break;
            case int16:
              snprintf(sendBuff, sendBuff_size, "%s %hd\n", s, *(int16_t *)(basepparam + cmdtab[i].offset));
              break;
            case int32:
              snprintf(sendBuff, sendBuff_size, "%s %ld\n", s, *(int32_t *)(basepparam + cmdtab[i].offset));
              break;
            }
            printf("GET: cmd=%s, %s\n", s, cmdtab[i].info);
          }
        }
      }
    }
  }
  if (cmd_not_found) {
    printf("ERROR: Command not found: %s\n", s);
    sprintf(sendBuff, "%s%s unknown-command\n", s, (is_set_cmd ? "!" : ""));
  }
}

