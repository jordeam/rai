#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <ctype.h>
#include <stdint.h>
#include <stddef.h>
#include <pthread.h>
#include <getopt.h>

#include "interpreter.h"
#include "oper_parameters.h"
#include "commands.h"
#include "interrupt.h"

#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

/* Buffers are declared static */
char sendBuff[BUFSIZ];
char recvBuff[BUFSIZ];

static int port = 5005;

void parse_opts(int argc, char **argv) {
  int c;

  while (1)
    {
      static struct option long_options[] =
        {
          {"port",  required_argument, 0, 'p'},
          {"end-time",  required_argument, 0, 't'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "t:p:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            break;
          printf ("option %s", long_options[option_index].name);
          if (optarg)
            printf (" with arg %s", optarg);
          printf ("\n");
          break;

        case 't':
          printf ("Setting end time `%s'\n", optarg);
          end_time = atof(optarg);
          break;

        case 'p':
          printf ("Using TCP Port `%s'\n", optarg);
          port = atoi(optarg);
          break;

        case '?':
          /* getopt_long already printed an error message. */
          break;

        default:
          abort ();
        }
    }

  /* Print any remaining command line arguments (not options). */
  if (optind < argc)
    {
      printf ("non-option ARGV-elements: ");
      while (optind < argc)
        printf ("%s ", argv[optind++]);
      putchar ('\n');
    }
}

int main(int argc, char *argv[]) {
  int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr; 

  parse_opts(argc, argv);
  
  oper_parameters_init();
  interrupt_init();
    
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  
  printf("Listening port %d\n", port);
  
  memset(&serv_addr, '0', sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port); 

  bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

  listen(listenfd, 10); 
  connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
  for(;;) {
    int n;
    n = read(connfd, recvBuff, BUFSIZ - 1);
    if (n == 0) {
      printf("INFO: got an empty command, maybe the socket is closed\n");
      printf("INFO: close socket connfd\n");
      close(connfd);
      printf("INFO: accept\n");
      connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
    }
    else {
      /* TODO n can be equal to BUFSIZ - 1*/
      recvBuff[n] = '\0';
      strtrim2(recvBuff);
      /* printf("recv: [%s]\n", recvBuff); */
      /* clear sebdBuff */
      *sendBuff = '\0';
      interpreter(recvBuff, sendBuff, BUFSIZ, baseaddr_params, cmdtab);
      write(connfd, sendBuff, strlen(sendBuff)); 
    }
  }
  return 0;
}
