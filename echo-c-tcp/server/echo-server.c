#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h> /* optarg */
#include <netinet/in.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>
#include "echo-server.h"
#include "../common.h"

struct settings settings;

typedef struct thread_data {
  int acceptedSocket;
  struct sockaddr_in clientAddress;
  char *message;
} threadData;

void *threadFunc(void *arg) {
        
    long numMessages = 0;
    threadData *data = (threadData *) arg;

    char addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(data->clientAddress.sin_addr), addr, INET_ADDRSTRLEN); 
    printf("Connection accepted from: %s \n", addr);


    int numbytes = 0;
    uint32_t length, nlength, id, nid;
    
    for(;;) {
      /* get message length */
      if((numbytes = read(data->acceptedSocket, &nlength, 4)) == -1) {
        perror("recv message size failure");
        close(data->acceptedSocket);
        pthread_exit(NULL);
      }
      /* check if last message is sent */
      if(ntohl(nlength) == 0) break;
      
      /* get message id */
      if((numbytes = read(data->acceptedSocket, &nid, 4)) == -1) {
        perror("recv message size failure");
        close(data->acceptedSocket);
        pthread_exit(NULL);
      }
      id = ntohl(nid);
      if(settings.verbose) printf("message %lu received\n", id);

      /* get message */
      char *msg;
      length = ntohl(nlength);
      msg = malloc(length + 1);
      if(settings.verbose) printf("length of message %lu \n", length);
      
      if((numbytes = read(data->acceptedSocket, msg, length)) == -1) {
        printf("%lu messages received\n", numMessages);
        perror("recv message failure");
        close(data->acceptedSocket);
        pthread_exit(NULL);
      }
      msg[length] = 0;
      numMessages++;

      /* send message id back as acknowledgement */
      if(settings.verbose) printf("sending id back %lu \n", ntohl(nid));
      
      if((numbytes = write(data->acceptedSocket, &nid, 4)) == -1) {
        perror("send failure");
        close(data->acceptedSocket);
        pthread_exit(NULL);
      }
    }
    printf("%lu messages received\n", numMessages); 
    close(data->acceptedSocket);
        
    pthread_exit(NULL);
}

int echo_server(char *host, int port, char *message) {

  int rc;
  struct sockaddr_in serverAddress, clientAddress;

  int socketDescriptor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  bzero((void *)&serverAddress, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;                            
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);                               
  serverAddress.sin_port = htons(port);

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
    threadData data = {acceptedSocketDescriptor, clientAddress, message};

    if((rc = pthread_create(&pth, NULL, threadFunc, &data))) {
      perror("failure creating thread");
      close(acceptedSocketDescriptor);
      continue;
    }

    //pthread_join(pth, NULL);
  }
}

static void usage(void) {
  printf("-i <ip address/hostname>      server to connect to (default: 0.0.0.0)\n"
      "-p <port number>              port number to connect on (default: 4242)\n"
      "-m <message>                  message to send (default: recv)\n"
      );
  return;
}

static void settings_init(void) {
  settings.host = "0.0.0.0";
  settings.port = 4242;
  settings.message = "recv";
  settings.verbose= false;
}

int main(int argc, char **argv) {

  int c;

  settings_init();

  while (-1 != (c = getopt(argc, argv,
          "h:"
          "i:"
          "p:"
          "m:"
          "c:"
          "b:"
          "v"
          ))) {
    switch (c) {
      case 'h':
        usage();
        exit(EXIT_SUCCESS);
      case 'i':
        settings.host = optarg;
        break;
      case 'p':
        settings.port = atoi(optarg);
        break;
      case 'm':
        settings.message = optarg;
        break;
      case 'v':
        settings.verbose = true;
        break;
      default:
        fprintf(stderr, "Illegal argument \"%c\"\n", c);
        exit(EXIT_FAILURE);
    }
  }
  echo_server(settings.host, settings.port, settings.message);
}
