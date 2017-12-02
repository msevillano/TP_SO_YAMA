/*
 * servidor.h
 *
 *  Created on: 4/9/2017
 *      Author: utnso
 */


#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "serializacion.h"
#include "mainYAMA.h"
#include "yama.h"
#include <protocoloComunicacion.h>

#define PORT 9034   // puerto en el que escuchamos

fd_set master;   // conjunto maestro de descriptores de fichero

void yrecv(int socket, void* buffer, int size, int flags);
void ysend(int socket, void* buffer, int size, int flags);
void inicializarServer();

#endif /* SERVIDOR_H_ */
