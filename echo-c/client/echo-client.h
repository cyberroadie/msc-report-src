#ifndef ECHO_CLIENT_H
#define ECHO_CLIENT_H

struct settings {
    int port;
    char *host;
    char *message; /* message to send to server */
    long count; /* number of messages to send */
};

struct audit {
  long success;
  long failure;
};

#endif
