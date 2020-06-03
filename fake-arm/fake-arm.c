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

#include "interpreter.h"
#include "parameters.h"
#include "commands.h"
#include "interrupt.h"

#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

#define MAX_COUNTER 1000

/* Buffers are declared static */
char sendBuff[BUFSIZ];
char recvBuff[BUFSIZ];

int main(int argc, char *argv[]) {
  int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr; 
  int port = 5000;
  
  init_parameters();
  init_interrupt();
  
  pthread_t trd;
  int inter_data = MAX_COUNTER;
  pthread_create(&trd, NULL, (void*) interval_code, &inter_data);

    
  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  if (argc == 2)
    port = atoi(argv[1]);

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
  free(sendBuff);

  pthread_join(trd, NULL);

  return 0;
}
