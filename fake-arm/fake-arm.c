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
#include <sys/ioctl.h>

#include "interpreter.h"
#include "oper_parameters.h"
#include "commands.h"
#include "flux_control.h"

#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

extern int ventilator_run;

/* Buffers are declared static */
char sendBuff[BUFSIZ];
char recvBuff[BUFSIZ];

static int port = 5005;

int isClosed(int sock) {
  int retval = 0;
  int bytestoread=0;
  fd_set myfd;
  struct timeval timeout = {0, 10000000};
 
  FD_ZERO(&myfd);
  FD_SET(sock,&myfd);
  int sio=select(FD_SETSIZE,&myfd, (fd_set *) 0,(fd_set *) 0, &timeout);
  //have to do select first for some reason
  /* int dio= */ioctl(sock, FIONREAD, &bytestoread);//should do error checking on return value of this
  retval=((bytestoread==0)&&(sio==1));
 
  return retval;
}

void parse_opts(int argc, char **argv) {
  int c;

  while (1)
    {
      static struct option long_options[] =
        {
         {"run", no_argument, &ventilator_run, 1},
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
  for(;;) {
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
    for(; !isClosed(connfd);) {
      int n;
      n = read(connfd, recvBuff, BUFSIZ - 1);
      /* TODO n can be equal to BUFSIZ - 1*/
      recvBuff[n] = '\0';
      strtrim2(recvBuff);
      /* printf("recv: [%s]\n", recvBuff); */
      /* clear sebdBuff */
      *sendBuff = '\0';
      interpreter(recvBuff, sendBuff, BUFSIZ, baseaddr_params, cmdtab);
      write(connfd, sendBuff, strlen(sendBuff));
    }
    close(connfd);
  }
  return 0;
}
