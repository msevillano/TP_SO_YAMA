/*
 * chat.c
 *
 *  Created on: 3/9/2017
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <archivos.h>
#include "mainYAMA.h"

typedef struct {
	int tipoMensaje;
	char mensaje[100];
} __attribute__((packed))
mensajeCorto;

void enviarMensajeCorto(int sockfd){
	mensajeCorto msj;
	msj.tipoMensaje = 2;
	printf("Escribe algo: ");
	memset(&msj.mensaje, 0, 100);
	//gets(msj.mensaje);

	void* mensajeAEnviar = (void*) malloc(sizeof(mensajeCorto));
	memset(mensajeAEnviar, 0, sizeof(mensajeCorto));
	memcpy(mensajeAEnviar, &msj, sizeof(mensajeCorto));

	if (send(sockfd, mensajeAEnviar, sizeof(mensajeCorto), 0) ==-1)
		printf("No puedo enviar\n");
	free (mensajeAEnviar);
}


void escribir(int* socket){
	int sockfd = *socket;
	while(1){
		enviarMensajeCorto(sockfd);
	}
}

void escuchar(int* socket){
	int sockfd = *socket;
	int buf;

	int nbytesReceived = 1;
	while(nbytesReceived != 0){		//Tanto FS, como memoria, y cpu saben solo escuchar mensajes por el momento, asi q quedamos en bucle hasta la muerte.
		buf = 0;

		if((nbytesReceived = recv(sockfd, &buf, sizeof(int), 0)) <= 0)				//recibo y compruebo q recibí correctamente
			printf("No puedo recibir información, o el servidor colgó\n");

		else manejarDatos(buf, sockfd);
	}
}

/*
void escuchar_chat(int socket){
	int rc;
	pthread_t tid;
	rc = pthread_create(&tid, NULL, escuchar, &socket);
		if(rc) printf("no pudo crear el hilo");
}

void escribir_chat(int socket){
	int rc;
	pthread_t tid;
	rc = pthread_create(&tid, NULL, escribir, &socket);
		if(rc) printf("no pudo crear el hilo");
}
*/
