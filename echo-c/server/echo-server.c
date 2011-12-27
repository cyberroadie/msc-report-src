#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <netinet/in.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include "../common.h"

typedef struct thread_data {
  int acceptedSocket;
  struct sockaddr_in clientAddress;
} threadData;

void *threadFunc(void *arg) {
        
    long numMessages = 0;
    threadData *data = (threadData *) arg;

    char buffer[16];
    inet_ntop(AF_INET, &data->clientAddress.sin_addr.s_addr, &buffer, sizeof(buffer));
    printf("Connection accepted from: %s \n", &buffer);

    char *msg;

    int total_numbytes = 0;
    int numbytes = 0;
    uint32_t length, nlength;
    
    for(;;) {
      if((numbytes = recv(data->acceptedSocket, &nlength, 4, 0)) == -1) {
        perror("recv message size failure");
        close(data->acceptedSocket);
        pthread_exit(NULL);
      }

      length = ntohl(nlength);
      msg = malloc(length + 1);
      if((numbytes = recv(data->acceptedSocket, msg, length, 0)) == -1) {
        perror("recv message failure");
        close(data->acceptedSocket);
        pthread_exit(NULL);
      }
      msg[length] = 0;

      if(strcmp(msg, CLOSE_MESSAGE) == 0) break;
      numMessages++;

      char rtn_msg[1024];
      rtn_msg[0] = '\0';
      strcat(rtn_msg, msg);
      strcat(rtn_msg, " received");
      if((numbytes = send(data->acceptedSocket, rtn_msg, sizeof(rtn_msg), 0)) == -1) {
        perror("send failure");
        close(data->acceptedSocket);
        pthread_exit(NULL);
      }
    }
    printf("%i messages received\n", numMessages); 
    close(data->acceptedSocket);
        
    pthread_exit(NULL);
}

int main() {

  int flags, in, rc;
  struct sockaddr_in serverAddress, clientAddress;

  int socketDescriptor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  bzero((void *)&serverAddress, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;                            
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);                               
  serverAddress.sin_port = htons(4342);

  if(-1 == bind(socketDescriptor, 
                (struct sockaddr *)&serverAddress, 
                sizeof(serverAddress))) {
    perror("bind error");
    close(socketDescriptor);
    exit(EXIT_FAILURE);
  }

  if(-1 == listen(socketDescriptor, 5)) {
    perror("listen error");
    close(socketDescriptor);
    exit(EXIT_FAILURE);
  }

  socklen_t len = sizeof(clientAddress);

  for(;;) {
    int acceptedSocketDescriptor = 
          accept(socketDescriptor, 
                 (struct sockaddr *) &clientAddress, 
                 &len);

    if(acceptedSocketDescriptor == -1) {
      perror("failure accepting socket");
      close(acceptedSocketDescriptor);
      continue;
    }
    
    pthread_t pth;
    threadData *data = {acceptedSocketDescriptor, clientAddress};

    if(rc = pthread_create(&pth, NULL, threadFunc, &data)) {
      perror("failure creating thread");
      close(acceptedSocketDescriptor);
      continue;
    }

    pthread_join(pth, NULL);
    
  }
}
