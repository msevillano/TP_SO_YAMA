#include "serializacionWorker.h"
#include "transformacionWorker.h"
#include "reduccionWorker.h"
#include <protocoloComunicacion.h>
#include "RGWorker.h"
#include <sockets.h>
#include "worker.h"
#include "enviarArchivo.h"

void _almacenamiento(int socket);

void manejarDatos(int buf, int socket){
	switch(buf){
	case TRANSFORMACIONWORKER:
		iniciarTransformacion(socket);
		break;
	case REDUCLOCAL:
		iniciarReduccion(socket);
		break;
	case SOLICITUDREDUCCIONGLOBAL:
		iniciarGlobal(socket);
		break;
	case ALMACENAMIENTO:
		_almacenamiento(socket);
		break;
    case SOLICITUDARCHIVOWORKER:
    	rutinaNoEncargado(socket);
        break;
    case RECEPCIONARCHIVOWORKER:
    	break;
	case FILESYSTEMOK:
		break;
	case FILESYSTEMERROR:
		break;
	}
}

void _almacenamiento(int socket){
	char archivo[255];
	char archivoFS[255];

	zrecv(socket, archivo, sizeof(char) * 255, 0);
	zrecv(socket, archivoFS, sizeof(char) * 255, 0);

	log_info(logger, "[ALMACENAMIENTO] Archivo a enviar al FS: %s", archivo);

	int mensaje = 4;
	zsend(socketFS, &mensaje, sizeof(mensaje), 0);

	zsend(socketFS, archivoFS, sizeof(char) * 255, 0);

	enviarArchivo(socketFS, archivo);

	int respuesta;

	int status;
	if(recv(socketFS, &status, sizeof(int), MSG_WAITALL) == -1 || status != 0){
		zsend(socket, &respuesta, sizeof(respuesta), 0);
		log_error(logger, "[ALMACENAMIENTO] Fallo almacenamiento de %s en el FS", archivoFS);
	}else{
		int respuesta = 0;
		zsend(socket, &respuesta, sizeof(respuesta), 0);
		log_info(logger, "[ALMACENAMIENTO] %s fue almacenado correctamente.", archivoFS);
	}
	exit(1);
}
