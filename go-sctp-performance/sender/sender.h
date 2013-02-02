#ifndef SENDER_H
#define SENDER_H

struct settings {
    int port;
    char *host;
    char *message; /* message to send to server */
    long count; /* number of messages to send */
    int no_of_threads;
    int bufsize;
    bool verbose;
    bool sctp;
};

struct audit {
  long success;
  long failure;
};

#endif
