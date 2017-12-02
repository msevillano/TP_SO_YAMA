#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "serializacionWorker.h"
#include "servidor.h"
#include "mainWorker.h"
#include <protocoloComunicacion.h>
#include "worker.h"

int socketFork;
int listener;    // descriptor de socket a la escucha


void comprobarConexion(int numbytes, int socket){
	 if (numbytes <= 0) {
		// error o conexión cerrada por el cliente
		if (numbytes == 0) {
			// conexión cerrada
			printf("selectserver: socket %d hung up\n", socket);
		} else {
			perror("recv");
		}
		close(socket); // bye!
	 }
}

void manejarCliente(int newfd){

	int numbytes;
	int buf;

	buf = 0;
	numbytes = recv(newfd, &buf, sizeof(int), 0); //leo el primer byte. Me dirá el tipo de paquete. (es un int)

	comprobarConexion(numbytes, newfd); //Me fijo si lo que recibí esta todo ok.

	manejarDatos(buf, newfd); //Si llegamos hasta acá manejamos los datos que recibimos.
}


void setServer(){

	struct sockaddr_in remoteaddr; // dirección del cliente
	int addrlen;
	setsid();

	// obtener socket a la escucha
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			perror("socket");
			exit(1);
		}
		// obviar el mensaje "address already in use" (la dirección ya se está usando)
		int yes=1;        // para setsockopt() SO_REUSEADDR, más abajo
		if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,
															sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		// enlazar
		struct sockaddr_in myaddr;     // dirección del servidor
		myaddr.sin_family = AF_INET;
		myaddr.sin_addr.s_addr = INADDR_ANY;
		myaddr.sin_port = htons(config_get_int_value(config, PUERTO_WORKER));
		memset(&(myaddr.sin_zero), '\0', 8);
		if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
			perror("bind");
			exit(1);
		}
		// escuchar
		if (listen(listener, 10) == -1) {
			perror("listen");
			exit(1);
		}

		for(;;) {  // main accept() loop
			addrlen = sizeof(struct sockaddr_in);
			if ((socketFork = accept(listener, (struct sockaddr *)&remoteaddr,
														   &addrlen)) == -1) {
				perror("accept");
				continue;
			}
			/*printf("server: got connection from %s\n",
											   inet_ntoa(remoteaddr.sin_addr));*/
			if (!fork()) { // Este es el proceso hijo
				close(listener); // El hijo no necesita este descriptor

				manejarCliente(socketFork);

				close(socketFork);
				exit(0);
			}
			close(socketFork);  // El proceso padre no lo necesita
		}
}
