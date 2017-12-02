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
#include "servidor.h"
#include "chat.h"
#include "conexionesFileSystem.h"
#include "fileSystem.h"
#include "serializacionFileSystem.h"
#include <sockets.h>
#include "funcionesConsolaFS.h"
#include "envioBloques.h"

void recibirArchivo(int socket, char * ruta);

void manejarStructs(int socket){
	//por ahora manejamos solo mensajes
	leerMensaje(socket);
}

void manejarDatos(int buf, int socket){
	switch(buf) {
		case OK:
			log_debug(logger, "Socket %i dice OK\n", socket);
			break;
		case CONEXIONNODO:
			asociarNodo(socket);
			break;
		case SOLICITUDARCHIVOYAMA:
			responderYAMA(socket);
			break;
		case RECEPCIONBLOQUE:
			recibirArchivo(socket, "tempfileAlmacenamiento");
			break;
		case RECEPCIONARCHIVOWORKER:
			break;
		case ERRORGUARDARBLOQUE:
			break;
		case CONEXIONYAMA:
			if (!fileSystemEstable) {
				int respuesta = -1;
				zsend(socket, &respuesta, sizeof(respuesta), 0);
				close(socket);
				FD_CLR(socket, &master); // eliminar del conjunto maestro
			} else{
				int respuesta = 0;
				zsend(socket, &respuesta, sizeof(respuesta), 0);
			}
			break;
	}
}

void recibirArchivo(int socket, char * ruta){
	int longitud;
	char archivoFS[255];

	zrecv(socket, archivoFS, sizeof(char) * 255, 0);
	zrecv(socket, &longitud, sizeof(longitud), 0);

	FILE * file = fopen(ruta, "w");

	while(longitud > MB){
		char buffer[MB];
		memset(buffer, 0, MB);
		zrecv(socket, buffer, MB * sizeof(char), 0);

		if(fwrite(buffer, sizeof(char) * MB, 1, file) != 1){
			printf("Error guardando el archivo\n");
		}

		longitud -= MB;
	}

	if(longitud > 0){
		char buffer[longitud];
		memset(buffer, 0, longitud);
		zrecv(socket, buffer, longitud * sizeof(char), 0);

		if(fwrite(buffer, sizeof(char) * longitud, 1, file) != 1){
			printf("Error guardando el archivo\n");
		}
	}

	fclose(file);

	int respuesta = cpfrom(ruta, archivoFS);

	zsend(socket, &respuesta, sizeof(respuesta), 0);
}