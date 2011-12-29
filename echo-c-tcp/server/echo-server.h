#ifndef ECHO_SERVER_H
#define ECHO_SERVER_H

struct settings {
    int port;
    char *host;
    char *message; /* answer to send to client */
};

#endif 
