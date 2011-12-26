#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <netinet/in.h> 
#include <sys/types.h>
#include <sys/socket.h>

int main() {

  int flags, in;
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
    
    char buffer[16];
    inet_ntop(AF_INET, &clientAddress.sin_addr.s_addr, &buffer, sizeof(buffer));
    printf("Connection accepted from: %s \n", &buffer);

    int numbytes = 0;
    char msg[6];
    if((numbytes = recv(acceptedSocketDescriptor, &msg, sizeof(msg), 0)) == -1) {
      perror("recv failure");
      close(acceptedSocketDescriptor);
      continue;
    }

    printf("%i bytes received\n", numbytes); 
    msg[numbytes] = '\0';
    puts(msg);

    char rtn_msg[1024];
    rtn_msg[0] = '\0';
    strcat(rtn_msg, msg);
    strcat(rtn_msg, " world!");
    puts(rtn_msg);
    if((numbytes = send(acceptedSocketDescriptor, rtn_msg, sizeof(rtn_msg), 0)) == -1) {
      perror("send failure");
      close(acceptedSocketDescriptor);
      continue;
    }
    printf("%i bytes sent\n", numbytes); 
    close(acceptedSocketDescriptor);
  }
}
