#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/sctp.h>
#include <unistd.h>
#include "echoServer.h"

#define RECVBUFSIZE 2048
#define SCTP_CONTROL_VEC_SIZE_RCV  16384 // lib/libc/net/sctp_sys_calls.c

struct settings settings;

static void usage(void) {
  printf("-i <ip address/hostname>      server to connect to (default: 0.0.0.0)\n"
         "-p <port number>              port number to connect on (default: 4242)\n"
         "-m <message>                  message to send (default: recv)\n"
      );
  return;
}

int echoServer(char *host, int port, char *message) {

  int sd;

  sd = socket(PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
  
  if(sd == -1) {
    perror("failure opening socket");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in serverAddress;
  bzero((void *)&serverAddress, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;                   
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);    
  serverAddress.sin_port = htons(port);

  if(-1 == bind(sd, (struct sockaddr *)&serverAddress, sizeof(serverAddress))) {
    perror("bind error");
    close(sd);
    exit(EXIT_FAILURE);
  }

  struct sctp_initmsg initMsg;
  bzero((void *)&initMsg, sizeof(initMsg));
  initMsg.sinit_num_ostreams = 100;
  initMsg.sinit_max_instreams = 100;
  initMsg.sinit_max_attempts = 0;
  initMsg.sinit_max_init_timeo = 0;

  setsockopt(sd, IPPROTO_SCTP, SCTP_INITMSG, &initMsg, sizeof(initMsg));
  setsockopt(sd, IPPROTO_SCTP, SCTP_LISTEN_FIX, 0, sizeof(SCTP_LISTEN_FIX));

  if(listen(sd, 1) < 0) {
    perror("failed to lisen for connection");
    exit(EXIT_FAILURE);
  }

  printf("listening on port %d\n", port);

  struct iovec iov[1];
  char buf[RECVBUFSIZE];
  iov->iov_base = buf;
  iov->iov_len = RECVBUFSIZE;

  struct sockaddr_in from;
  socklen_t *fromlen = NULL, infolen;
  int flags = 0;
  unsigned int infotype = 0;
  struct sctp_rcvinfo rinfo;
  bzero(&rinfo, sizeof(struct sctp_rcvinfo));
  infolen = sizeof(rinfo);

  int on = 1;
  if (setsockopt(sd, IPPROTO_SCTP, SCTP_RECVRCVINFO, &on, sizeof(on)) < 0) {
    perror("setsockopt SCTP_RECVRCVINFO");
    exit(EXIT_FAILURE);
  }
  
  struct sctp_sndinfo sinfo;
  bzero(&sinfo, sizeof(struct sctp_sndinfo));

 
  for(;;) {
    int length = sctp_recvv(sd, 
                            iov, 
                            1, 
                            (struct sockaddr *) &from, 
                            fromlen, 
                            &rinfo, 
                            &infolen, 
                            &infotype,
                            &flags);
    
    if(length == -1) {
      perror("error receiving message: ");
      continue;
    }
   
    buf[length] = '\0'; 
    printf("message received: %s\n", buf);
    
    printf("[ Receive echo (%u bytes): stream = %hu, "
        "flags = %hx, ppid = %u ]\n", length,
        rinfo.rcv_sid, rinfo.rcv_flags,
        ntohl(rinfo.rcv_ppid));


    sinfo.snd_sid = rinfo.rcv_sid;
    sinfo.snd_flags = rinfo.rcv_flags;
    sinfo.snd_ppid = rinfo.rcv_ppid;
    sinfo.snd_assoc_id = rinfo.rcv_assoc_id;

    printf("sending message back: %s\n", buf);
    
    sctp_sendv(sd, 
        iov,
        1,
        (struct sockaddr *) &from,
        1,
        (void *) &sinfo,
        sizeof(struct sctp_sndinfo),
        SCTP_SENDV_SNDINFO,
        0);

  }

  return EXIT_SUCCESS;
}

static void settingsInit(void) {
  settings.host = "0.0.0.0";
  settings.port = 4242;
  settings.message = "recv";
  settings.verbose= false;
}

int main(int argc, char **argv) {
 
  int c;

  settingsInit();

  while(-1 != (c = getopt(argc, argv,
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
  echoServer(settings.host, settings.port, settings.message);
}
