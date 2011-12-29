#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* optarg */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h> /* TCP_NODELAY lives here */
#include <stdbool.h>
#include "echo-client.h"
#include "../common.h"

struct settings settings;
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
  uint32_t nlength, nid, ack_id, ack_nid;
  ssize_t numbytes;

  printf("start sending %lu messages\n", count);

  for(uint32_t id = 0; id < count; id++) {

    if(settings.verbose) printf("sending message %lu\n", id);

    // send size of message
    length = strlen(message);
    nlength = htonl(length);

    if((numbytes = send(socketDescriptor, &nlength, 4, 0)) == -1) {
      perror("send message size failure");
      audit.failure++;
      break;
    } 

    // send message id
    nid = htonl(id);
    if((numbytes = send(socketDescriptor, &nid, 4, 0)) == -1) {
      perror("send message id failure");
      audit.failure++;
      break;
    }

    // send message
    if((numbytes = send(socketDescriptor, message, length, 0)) == -1) {
      perror("send message failure");
      audit.failure++;
      break;
    }

    /* get response (message id) */
    if((numbytes = recv(socketDescriptor, &ack_nid, 4, 0)) == -1) {
      perror("recv id failure");
      audit.failure++;
      break;
    }

    ack_id = ntohl(ack_nid);
    if(ack_id != id) {
      printf("ack_id: %lu != id: %lu\n", ack_id, id);
      break;
    }

    audit.success++;
  }
 

  /* close the message stream by sending a zero length message */
  length = 0;
  nlength = htonl(length);
  if((numbytes = send(socketDescriptor, &nlength, 4, 0)) == -1) {
      perror("send message size failure");
      close(socketDescriptor);
      exit(EXIT_FAILURE);
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

static void settings_init(void) {
  settings.host = "127.0.0.1";
  settings.port = 4242;
  settings.message = "hello";
  settings.count = 1;
  settings.bufsize = 4096;
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
      case 'c':
        settings.count = atol(optarg);
        break;
      case 'b':
        settings.bufsize = atoi(optarg);
        break;
      case 'v':
        settings.verbose = true;
        break;
      default:
        fprintf(stderr, "Illegal argument \"%c\"\n", c);
        exit(EXIT_FAILURE);
    }
  }
  echo_client(settings.host, settings.port, settings.message, settings.count);
}


