
#ifndef COMMONS_SOCKETS_H_
#define COMMONS_SOCKETS_H_

#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <unistd.h>
#include <commons/collections/list.h>
#include <stdlib.h>
#include <string.h>
#include "string.h"
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>


int zrecv(int socket, void* buffer, int size, int flags);
int zsend(int socket, void* buffer, int size, int flags);

int sendAll(int fd, char *cosa, int size, int flags);
int recvAll(int fd, char *buffer, int size, int flags);

#endif /* COMMONS_SOCKETS_H_ */
