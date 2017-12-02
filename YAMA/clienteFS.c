#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sockets.h>
#include <arpa/inet.h>
#include "clienteFS.h"
#include "yama.h"
#include <protocoloComunicacion.h>

void doConnect(int *sockfd);

int CONEXIONYAMA = 10;

void iniciarConexionAFS(int *sockfd){
	int respuesta = -1;

	while(respuesta == -1){
		doConnect(sockfd);

		zrecv(*sockfd, &respuesta, sizeof(respuesta), 0);

		if(respuesta == -1){
			log_error(logger, "File System no aceptó la conexion\n");
			sleep(3);
			log_info(logger, "Reintentando conexion a File System");
		}else{
			log_info(logger, "Conexion exitosa a File System\n");
		}
	}
}

void doConnect(int *sockfd){
	struct sockaddr_in their_addr; // información de la dirección de destino


	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
	their_addr.sin_port = conexionFileSystem.puerto;  // short, Ordenación de bytes de la red
	their_addr.sin_addr.s_addr = inet_addr(conexionFileSystem.ip);
	memset(&(their_addr.sin_zero), 0, 8);  // poner a cero el resto de la estructura

	if (connect(*sockfd, (struct sockaddr *)&their_addr,
				sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}

	zsend(*sockfd, &CONEXIONYAMA, sizeof(CONEXIONYAMA), 0);
}

