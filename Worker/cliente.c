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
#include "cliente.h"
#include <protocoloComunicacion.h>
#include <sockets.h>
#include "worker.h"

void iniciarConexionAServer(int* sockfd){

	struct sockaddr_in their_addr; // información de la dirección de destino


	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
	their_addr.sin_port = htons(config_get_int_value(config, PUERTO_FILESYSTEM));  // short, Ordenación de bytes de la red
	their_addr.sin_addr.s_addr = inet_addr(config_get_string_value(config, IP_FILESYSTEM));
	memset(&(their_addr.sin_zero), 0, 8);  // poner a cero el resto de la estructura

	if (connect(*sockfd, (struct sockaddr *)&their_addr,
				sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}
}

void conectarANodo(int* sockfd, char*ip, uint16_t puerto){

	struct sockaddr_in their_addr; // información de la dirección de destino


	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	log_info(logger, "Conectando a %s:%d\n", ip, puerto);
	their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
	their_addr.sin_port = puerto;  // short, Ordenación de bytes de la red
	their_addr.sin_addr.s_addr = inet_addr(ip);
	memset(&(their_addr.sin_zero), 0, 8);  // poner a cero el resto de la estructura

	if (connect(*sockfd, (struct sockaddr *)&their_addr,
				sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}
}
