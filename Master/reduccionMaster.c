#include "mainMaster.h"
#include "master.h"
#include <stdio.h>
#include <stdlib.h>
#include <archivos.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "chat.h"
#include "enviarArchivo.h"
#include "cliente.h"
#include "conexionesYAMA.h"
#include "transformacionMaster.h"
#include <protocoloComunicacion.h>
#include <sockets.h>

int conexionReduccionWorker(int *sockfd, OperacionReduccion op) {

	struct sockaddr_in their_addr; // información de la dirección de destino

	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return 1;
	}

	their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
	their_addr.sin_port = op.puerto;
	//their_addr.sin_addr.s_addr = inet_addr(t.ipWorker);
	their_addr.sin_addr.s_addr = inet_addr(op.ip);
	memset(&(their_addr.sin_zero), 0, 8); // poner a cero el resto de la estructura

	if (connect(*sockfd, (struct sockaddr *) &their_addr,
				sizeof(struct sockaddr)) == -1) {
		perror("connect");
		return 1;
	}
	return 0;
}

void * mandarSolicitudReduccion(OperacionReduccion* op) {
	int socketNodo;
	int tipoMensaje;
	struct timeval tv1, tv2;

	pthread_mutex_lock(&mutexReduccion);
	cantReduActual++;
	calcularMaximos();
	pthread_mutex_unlock(&mutexReduccion);
	gettimeofday(&tv1, NULL);

	if (conexionReduccionWorker(&socketNodo, *op)) {
		int mensajeError = FALLOREDLOCAL;
		log_error(logger, "Fallo reduccion en nodo %s", op->nombreNodo);
		pthread_mutex_lock(&mutexReduccion);
		cantReduActual--;
		fallosRedu++;
		pthread_mutex_unlock(&mutexReduccion);
		pthread_mutex_lock(&yamaMensajes);
		zsend(socket_yama, &mensajeError, sizeof(int), 0);
		zsend(socket_yama, &jobId, sizeof(jobId), 0);
		zsend(socket_yama, op->archivoReducido, sizeof(char) * 255, 0);
		pthread_mutex_unlock(&yamaMensajes);
	} else {
		tipoMensaje = PEDIDOREDUCCION;

		zsend(socketNodo, &tipoMensaje, sizeof(int), 0);
		zsend(socketNodo, &op->cantidadTemporales,
				sizeof(op->cantidadTemporales), 0);

		int i;
		for (i = 0; i < op->cantidadTemporales; ++i) {
			zsend(socketNodo, op->temporales[i], sizeof(char) * 255, 0);
		}

		zsend(socketNodo, op->archivoReducido, sizeof(char) * 255, 0);

		enviarArchivo(socketNodo, reductor);

		int status;
		if (recv(socketNodo, &status, sizeof(int), MSG_WAITALL) <= 0
				|| status != 0) {
			int mensajeError = FALLOREDLOCAL;
			log_error(logger, "Fallo reduccion en nodo %s", op->nombreNodo);
			pthread_mutex_lock(&mutexReduccion);
			cantReduActual--;
			fallosRedu++;
			pthread_mutex_unlock(&mutexReduccion);
			pthread_mutex_lock(&yamaMensajes);
			zsend(socket_yama, &mensajeError, sizeof(int), 0);
			zsend(socket_yama, &jobId, sizeof(jobId), 0);
			zsend(socket_yama, op->archivoReducido, sizeof(char) * 255, 0);
			pthread_mutex_unlock(&yamaMensajes);

		} else {
			int mensajeOK = REDLOCALOK;
			log_info(logger, "Worker %d finalizó reduccion", socketNodo);
			gettimeofday(&tv2, NULL);

			double tiempoTotal = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000
					+ (double) (tv2.tv_sec - tv1.tv_sec);

			pthread_mutex_lock(&mutexReduccion);
			tiempoTotalRedu += tiempoTotal;
			reduccionesOk++;
			cantReduActual--;
			pthread_mutex_unlock(&mutexReduccion);
			pthread_mutex_lock(&yamaMensajes);
			zsend(socket_yama, &mensajeOK, sizeof(int), 0);
			zsend(socket_yama, &jobId, sizeof(jobId), 0);
			zsend(socket_yama, op->archivoReducido, sizeof(char) * 255, 0);
			pthread_mutex_unlock(&yamaMensajes);
		}
	}

	free(op);
	pthread_exit(NULL);

	return NULL;
}

void reduccionLocal() {
	OperacionReduccion * op = (OperacionReduccion *) malloc(sizeof(*op));

	zrecv(socket_yama, op->nombreNodo, sizeof(char) * 100, 0);
	zrecv(socket_yama, op->ip, sizeof(char) * 20, 0);
	zrecv(socket_yama, &op->puerto, sizeof(op->puerto), 0);
	zrecv(socket_yama, &op->cantidadTemporales, sizeof(op->cantidadTemporales), 0);

	int i;

	log_info(logger, "Reduccion a Realizar: %s\tIP: %s\tPUERTO: %d", op->nombreNodo, op->ip, op->puerto);

	for(i = 0; i < op->cantidadTemporales; ++i){
		zrecv(socket_yama, op->temporales[i], sizeof(char) * 255, 0);

		log_info(logger, "\t-> TEMPORAL %d: %s", i+1, op->temporales[i]);
	}

	zrecv(socket_yama, op->archivoReducido, sizeof(char) *  255, 0);

	pthread_t tid;

	pthread_create(&tid, NULL, (void*) mandarSolicitudReduccion, op);
}
