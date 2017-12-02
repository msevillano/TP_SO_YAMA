/*
 * main.h
 *
 *  Created on: 3/9/2017
 *      Author: utnso
 */

#ifndef MAINWORKER_H_
#define MAINWORKER_H_

typedef struct {
	int tipoMensaje;
	char mensaje[100];
} __attribute__((packed))
mensajeCorto;

void nuevoCliente(char* remoteHost, int newfd);
void manejarDatos(int buf, int socket);
void forkear(int socket_master);

#endif /* MAINWORKER_H_ */
