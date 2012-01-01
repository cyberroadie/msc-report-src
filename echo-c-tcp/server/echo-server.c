#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* optarg */
#include <netinet/in.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include "echo-server.h"
#include "../common.h"

struct settings settings;

typedef struct server_thread_data {
  int acceptedSocket;
  struct sockaddr_in client_address;
  char *message;
} server_thread_data_t;

void *server_thread(void *arg) {
        
    long numMessages = 0;
    server_thread_data_t *data = (server_thread_data_t *) arg;

    char addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(data->client_address.sin_addr), addr, INET_ADDRSTRLEN); 
    printf("Connection accepted from: %s \n", addr);

    int numbytes = 0;
    uint32_t length, nlength, id, nid;
    
    for(;;) {
      /* get message length */
      if((numbytes = recv(data->acceptedSocket, &nlength, 4, MSG_WAITALL)) == -1) {
        perror("recv message size failure");
        close(data->acceptedSocket);
        pthread_exit(NULL);
      }

      /* check if last message is sent */
      if(ntohl(nlength) == 0) break;
      
      /* get message id */
      if((numbytes = recv(data->acceptedSocket, &nid, 4, MSG_WAITALL)) == -1) {
        perror("recv message size failure");
        close(data->acceptedSocket);
        pthread_exit(NULL);
      }

      id = ntohl(nid);
      if(settings.verbose) printf("message %d received\n", id);

      /* get message */
      char *msg;
      length = ntohl(nlength);
      msg = malloc(length);
      if((numbytes = recv(data->acceptedSocket, msg, length, MSG_WAITALL)) == -1) {
        printf("%lu messages received\n", numMessages);
        perror("recv message failure");
        close(data->acceptedSocket);
        pthread_exit(NULL);

      }
      msg[length] = 0;
      numMessages++;

      /* send message id back as acknowledgement */
      if((numbytes = send(data->acceptedSocket, &nid, 4, 0)) == -1) {
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
  struct sockaddr_in serverAddress, client_address;

  int sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  bzero((void *)&serverAddress, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;                            
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);                               
  serverAddress.sin_port = htons(port);

  if(-1 == bind(sd, 
                (struct sockaddr *)&serverAddress, 
                sizeof(serverAddress))) {
    perror("bind error");
    close(sd);
    exit(EXIT_FAILURE);
  }

  if(-1 == listen(sd, 5)) {
    perror("listen error");
    close(sd);
    exit(EXIT_FAILURE);
  }

  socklen_t len = sizeof(client_address);

  server_thread_data_t *data;

  for(;;) {
    data = (server_thread_data_t*)malloc(sizeof(server_thread_data_t));
    data->message = strdup(message); 
    data->acceptedSocket = accept(sd, 
                                  (struct sockaddr *) &data->client_address, 
                                  &len);

    if(data->acceptedSocket == -1) {
      perror("failure accepting socket");
      close(data->acceptedSocket);
      continue;
    }
 
    pthread_t pth;

    if((rc = pthread_create(&pth, NULL, server_thread, data))) {
      perror("failure creating thread");
      close(data->acceptedSocket);
      continue;
    }
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
