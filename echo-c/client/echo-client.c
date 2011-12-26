#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>

int main() {
  
  int flags;
  struct sctp_initmsg initmsg;
  struct sockaddr_in serverAddress;

  bzero(&initmsg, sizeof(initmsg));
  
  // one-to-one sctp mode
  int socketDescriptor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  bzero((void *)&serverAddress, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET; // ipv4 for now
  serverAddress.sin_port = htons(4342);
  serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

  if(-1 == connect(socketDescriptor, 
                   (struct sockaddr *) &serverAddress, 
                   sizeof(serverAddress))) {
      perror("connection failure");
      close(socketDescriptor);
      exit(EXIT_FAILURE);  
  }

  int numbytes = 0;
  char *msg = "hello";
  if((numbytes = send(socketDescriptor, msg, strlen(msg) + 1, 0)) == -1) {
    perror("send failure");
    close(socketDescriptor);
    exit(EXIT_FAILURE);
  }

  printf("%i bytes sent\n", numbytes); 
  numbytes = 0;
  char buffer[1024];
  if((numbytes = recv(socketDescriptor, &buffer, sizeof(buffer), 0)) == -1) {
    perror("recv failure");
    exit(EXIT_FAILURE);
  }

  printf("%i bytes received\n", numbytes); 
  buffer[numbytes] = '\0';
  puts(buffer);

  close(socketDescriptor);
  exit(EXIT_SUCCESS);
}
