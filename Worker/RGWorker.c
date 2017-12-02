#include <protocoloComunicacion.h>
#include <stdint.h>
#include "cliente.h"
#include <sockets.h>
#include <stdio.h>
#include "RGWorker.h"
#include "worker.h"
#include "transformacionWorker.h"
#include "enviarArchivo.h"
#include "apareo.h"

int _ejecutarReduccionGlobal(char * rutaScript, char * rutaArchivo, char * rutaFinal);

void rutinaNoEncargado(int socket){
	char ruta[255];
	zrecv(socket, ruta, sizeof(char) * 255, 0);

	enviarArchivo(socket, ruta);

	//hago mi magia y muero (le envio el archivo)
	exit(0);
}

void iniciarGlobal(int mastersock){
	char archivoReducido[255];
	char archivoFinal[255];
	int cantidad;
	int i;

	zrecv(mastersock, archivoReducido, sizeof(char) * 255, 0);
	zrecv(mastersock, archivoFinal, sizeof(char) * 255, 0);

	log_info(logger, "[REDUCCION GLOBAL] Archivo de reduccion global %s", archivoFinal);

	zrecv(mastersock, &cantidad, sizeof(cantidad), 0);

	NodoGlobal nodos[cantidad];
	log_info(logger, "[REDUCCION GLOBAL] Solicitudes a homonimos %d", cantidad);

	for(i = 0; i < cantidad; ++i){
		zrecv(mastersock, nodos[i].nombre, sizeof(char) * 100, 0);
		zrecv(mastersock, nodos[i].ip, sizeof(char) * 20, 0);
		zrecv(mastersock, &nodos[i].puerto, sizeof(nodos[i].puerto), 0);
		zrecv(mastersock, nodos[i].archivoReducido, sizeof(char) * 255, 0);

		log_info(logger, "[REDUCCION GLOBAL] %s\tIP:%s\tPUERTO:%d\tARCHIVO:%s", nodos[i].nombre, nodos[i].ip, nodos[i].puerto, nodos[i].archivoReducido);
	}

	char ruta[255];
	pid_t pid = getpid();
	sprintf(ruta, "scripts/reduccionGlobal%d.sh", pid);

	recibirArchivo(mastersock, ruta);

	int socket_nodos[cantidad];
	char rutasTemporales[cantidad + 1][255];
	int pedArchivo = SOLICITUDARCHIVOWORKER;

	strcpy(rutasTemporales[0], archivoReducido);

	log_info(logger, "[REDUCCION GLOBAL] Solicitando archivos reducidos.");

	for(i = 0; i < cantidad; ++i){

		sprintf(rutasTemporales[i + 1], "reducciones/reduccion-%s-%d", nodos[i].nombre, pid);

		log_info(logger, "[REDUCCION GLOBAL] Conectando a %s", nodos[i].nombre);

		conectarANodo(&socket_nodos[i], nodos[i].ip, nodos[i].puerto);
		zsend(socket_nodos[i], &pedArchivo, sizeof(int), 0);
		zsend(socket_nodos[i], nodos[i].archivoReducido, sizeof(char) * 255, 0);

		recibirArchivo(socket_nodos[i], rutasTemporales[i + 1]);

		log_info(logger, "[REDUCCION GLOBAL] Recibido %s", nodos[i].archivoReducido);

		close(socket_nodos[i]);
	}

	char tempFile[255];
	strcpy(tempFile, archivoReducido); // copio el archivo final para el caso en que no hay otro worker al que conectarse

	for (i = 0; i < cantidad; ++i) {
		memset(tempFile, 0, sizeof(char)*255);
		sprintf(tempFile, "%s-%d", archivoFinal, i);
		apareo(rutasTemporales[i], rutasTemporales[i + 1], tempFile);
		strcpy(rutasTemporales[i + 1], tempFile);
	}
	log_info(logger, "[REDUCCION GLOBAL] Archivo reducido global: %s\n", tempFile);

	int ok = _ejecutarReduccionGlobal(ruta, tempFile, archivoFinal);
	zsend(mastersock, &ok, sizeof(ok), 0);

	close(mastersock);
}

int _ejecutarReduccionGlobal(char * rutaScript, char * rutaArchivo, char * rutaFinal){
	char command[512];
	sprintf(command, "cat %s | %s >> %s \n", rutaArchivo, rutaScript, rutaFinal);

	return system(command);
}
