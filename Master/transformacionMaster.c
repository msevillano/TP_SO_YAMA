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
#include "reduccionMaster.h"
#include <commons/collections/list.h>
#include <sys/time.h>

void mandarSolicitudTransformacion(workerTransformacion* t);

int respuestaSolicitud() {

	int cantidadWorkers = 0;

	zrecv(socket_yama, &jobId, sizeof(jobId), 0);
	log_info(logger, "Job Id: %d", jobId);

	zrecv(socket_yama, &cantidadWorkers, sizeof(int), 0);

	if(cantidadWorkers == 0){
		log_error(logger,
				 "[ABORTADO] El archivo %s no está disponible en el FS.", pathArchivoFS);
		exit(1);
	}

	return cantidadWorkers;
}

int conexionTransfWorker(int *sockfd, workerTransformacion t){

	struct sockaddr_in their_addr; // información de la dirección de destino


	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return(1);
	}

	log_info(logger, "ip: %s\tport: %d", t.ipWorker, t.puertoWorker);

	their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
	their_addr.sin_port = t.puertoWorker;  // short, Ordenación de bytes de la red
	their_addr.sin_addr.s_addr = inet_addr(t.ipWorker);
	memset(&(their_addr.sin_zero), 0, 8);  // poner a cero el resto de la estructura

	if (connect(*sockfd, (struct sockaddr *)&their_addr,
				sizeof(struct sockaddr)) == -1) {
		perror("connect");
		return(1);
	}
	return 0;
}

void mandarSolicitudTransformacionConFree(workerTransformacion* t){
	mandarSolicitudTransformacion(t);
	free(t);
}

void mandarSolicitudTransformacion(workerTransformacion* t) {
	int socketWorker;
	int tipoMensaje;
	struct timeval tv1, tv2;

	pthread_mutex_lock(&mutexTransformacion);
	cantTransfActual++;
	calcularMaximos();
	pthread_mutex_unlock(&mutexTransformacion);

	gettimeofday(&tv1, NULL);
	if (conexionTransfWorker(&socketWorker, *t)) {
		log_error(logger, "No se pudo conectar al worker. Replanificar.");

		int tipomensaje = FALLOTRANSFORMACION;
		transfError mensajeError;
		strcpy(mensajeError.nombreNodo, t->nombreNodo);
		strcpy(mensajeError.nombreTemp, t->rutaArchivo);
		mensajeError.numBloque = t->numBloque;
		mensajeError.bytes = t->bytesOcupados;
		pthread_mutex_lock(&mutexTransformacion);
		cantTransfActual--;
		fallosTransf++;
		pthread_mutex_unlock(&mutexTransformacion);
		pthread_mutex_lock(&yamaMensajes);
		zsend(socket_yama, &tipomensaje, sizeof(tipomensaje), 0);
		zsend(socket_yama, &mensajeError, sizeof(mensajeError), 0);
		pthread_mutex_unlock(&yamaMensajes);
	} else {

		mensajeTransformacion mensaje;
		tipoMensaje = 1;
		mensaje.cantidadBytes = t->bytesOcupados;
		mensaje.bloque = t->numBloque;
		strcpy(mensaje.nombreTemp, t->rutaArchivo);

		zsend(socketWorker, &tipoMensaje, sizeof(int), 0);
		zsend(socketWorker, &mensaje, sizeof(mensaje), 0);

		enviarArchivo(socketWorker, transformador);

		int status, bytes;
		bytes = recv(socketWorker, &status, sizeof(int), MSG_WAITALL);
		if (bytes <= 0 || status != 0) {
			int tipomensaje = FALLOTRANSFORMACION;
			transfError mensajeError;
			strcpy(mensajeError.nombreNodo, t->nombreNodo);
			strcpy(mensajeError.nombreTemp, t->rutaArchivo);
			mensajeError.numBloque = t->numBloque;
			mensajeError.bytes = t->bytesOcupados;
			log_error(logger, "Fallo la transformacion en el nodo %s",
					t->nombreNodo);
			pthread_mutex_lock(&mutexTransformacion);
			cantTransfActual--;
			fallosTransf++;
			pthread_mutex_unlock(&mutexTransformacion);
			log_error(logger, "Fallo la transformacion envio un %d",
					tipomensaje);
			pthread_mutex_lock(&yamaMensajes);
			zsend(socket_yama, &tipomensaje, sizeof(tipomensaje), 0);
			zsend(socket_yama, &mensajeError, sizeof(mensajeError), 0);
			pthread_mutex_unlock(&yamaMensajes);

		} else {
			int mensajeOK = TRANSFORMACIONOK;

			gettimeofday(&tv2, NULL);
			log_info(logger, "Worker %d finalizó transformación",
					socketWorker);

			double tiempoTotal = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000
					+ (double) (tv2.tv_sec - tv1.tv_sec);

			pthread_mutex_lock(&mutexTransformacion);
			tiempoTotalTransf += tiempoTotal;
			transformacionesOk++;
			cantTransfActual--;
			pthread_mutex_unlock(&mutexTransformacion);
			pthread_mutex_lock(&yamaMensajes);
			zsend(socket_yama, &mensajeOK, sizeof(int), 0);
			zsend(socket_yama, &jobId, sizeof(jobId), 0);
			zsend(socket_yama, mensaje.nombreTemp, sizeof(char) * 255, 0);
			pthread_mutex_unlock(&yamaMensajes);
		}
	}

	pthread_exit(NULL);
}

void mandarReplanificado(){
	pthread_t tid;
	int rc;
	workerTransformacion * t = (workerTransformacion *) malloc(sizeof(*t));
	zrecv(socket_yama, t->nombreNodo, sizeof(char) * 100, 0);
	zrecv(socket_yama, t->ipWorker, sizeof(char) * 20, 0);
	zrecv(socket_yama, &t->puertoWorker, sizeof(t->puertoWorker), 0);
	zrecv(socket_yama, &t->numBloque, sizeof(t->numBloque), 0);
	zrecv(socket_yama, &t->bytesOcupados, sizeof(t->bytesOcupados), 0);
	zrecv(socket_yama, t->rutaArchivo, 255 * sizeof(char), 0);

	rc = pthread_create(&tid, NULL, (void*) mandarSolicitudTransformacionConFree, t);

	if (rc)
		log_error(logger, "No pudo crear el hilo %d");
}

void mandarTransformacionNodo() {
	int cantidadWorkers = respuestaSolicitud();
	workerTransformacion t[cantidadWorkers];
	pthread_t tid[cantidadWorkers];
	int rc[cantidadWorkers];
	int i = 0;

	while (cantidadWorkers--) {
		zrecv(socket_yama, t[cantidadWorkers].nombreNodo, sizeof(char) * 100,
				0);
		zrecv(socket_yama, t[cantidadWorkers].ipWorker, sizeof(char) * 20, 0);
		zrecv(socket_yama, &t[cantidadWorkers].puertoWorker,
				sizeof(t[cantidadWorkers].puertoWorker), 0);
		zrecv(socket_yama, &t[cantidadWorkers].numBloque,
				sizeof(t[cantidadWorkers].numBloque), 0);
		zrecv(socket_yama, &t[cantidadWorkers].bytesOcupados,
				sizeof(t[cantidadWorkers].bytesOcupados), 0);
		zrecv(socket_yama, t[cantidadWorkers].rutaArchivo, 255 * sizeof(char),
				0);

		rc[cantidadWorkers] = pthread_create(&tid[cantidadWorkers], NULL,
				(void*) mandarSolicitudTransformacion, &t[cantidadWorkers]);

		if (rc[cantidadWorkers])
			log_error(logger, "No pudo crear el hilo %d", i);
		i++;
	}

	int operacion = 0;
	while (operacion != SOLICITUDREDUCCIONGLOBAL) {
		zrecv(socket_yama, &operacion, sizeof(operacion), 0);
		switch (operacion) {
			case REPLANIFICACION:
				mandarReplanificado();
				break;
			case SOLICITUDREDUCCIONLOCAL:
				reduccionLocal(socket_yama);
				break;
			case FALLOTRANSFORMACION:
				log_error(logger,
						"[ABORTADO] YAMA no pudo replanificar transformacion.");
				exit(1);
				break;
			case FALLOREDLOCAL:
				log_error(logger,
						"[ABORTADO] YAMA no puede replanificar una reduccion.");
				exit(1);
				break;
		}
	}
}
