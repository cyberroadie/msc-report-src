#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* optarg */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <netinet/tcp.h> /* TCP_NODELAY lives here */
#include "echo-client.h"
#include "../common.h"

struct audit audit;

void echo_client(char *host, int port, char *message, long count) {

  struct sockaddr_in serverAddress;
  audit.success = 0;
  audit.failure = 0;


  bzero((void *)&serverAddress, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET; 
  serverAddress.sin_port = htons(port);
  serverAddress.sin_addr.s_addr = inet_addr(host);

  int socketDescriptor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(-1 == connect(socketDescriptor, 
        (struct sockaddr *) &serverAddress, 
        sizeof(serverAddress))) {
    perror("connection failure");
    close(socketDescriptor);
    exit(EXIT_FAILURE);  
  }
  
  int flag = 1;
  setsockopt(socketDescriptor, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

 
  size_t length;
  uint32_t nlength;
  int numbytes;

  for(int i = 0; i < count; i++) {

    // send size of message
    length = strlen(message);
    nlength = htonl(length);

    if((numbytes = send(socketDescriptor, &nlength, 4, 0)) == -1) {
      perror("send message size failure");
      audit.failure++;
      continue;
    } 

    // send message
    if((numbytes = send(socketDescriptor, message, length, 0)) == -1) {
      perror("send message failure");
      audit.failure++;
      continue;
    }

    numbytes = 0;
    char buffer[1024];
    if((numbytes = recv(socketDescriptor, &buffer, sizeof(buffer), 0)) == -1) {
      perror("recv failure");
      audit.failure++;
      continue;
    }

    audit.success++;
  }
 
  length = strlen(CLOSE_MESSAGE);
  nlength = htonl(length);

  if((numbytes = send(socketDescriptor, &nlength, 4, 0)) == -1) {
      perror("send message size failure");
      exit(EXIT_FAILURE);
  }

  // send close message
  if(send(socketDescriptor, CLOSE_MESSAGE, length, 0) == -1) {
      perror("send close server socket message failure");
  }

  close(socketDescriptor);

  printf("%lu messages send succesful\n", audit.success); 
  printf("%lu messages failed\n", audit.failure); 

  exit(EXIT_SUCCESS);
}

static void usage(void) {
  printf("-i <ip address/hostname>      server to connect to (default: localhost)\n"
      "-p <port number>              port number to connect to (default: 4242)\n"
      "-m <message>                  message to send (default: hello)\n"
      "-c <count>                    number of messages to send (default: 1)\n"
      );
  return;
}

struct settings settings;

static void settings_init(void) {
  settings.host = "127.0.0.1";
  settings.port = 4242;
  settings.message = "hello";
  settings.count = 1;
  settings.bufsize = 4096;
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
      case 'c':
        settings.count = atol(optarg);
        break;
      case 'b':
        settings.bufsize = atoi(optarg);
        break;
      default:
        fprintf(stderr, "Illegal argument \"%c\"\n", c);
        exit(EXIT_FAILURE);
    }
  }
  echo_client(settings.host, settings.port, settings.message, settings.count);
}


