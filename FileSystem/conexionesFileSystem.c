#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "utilidadesFileSystem.h"
#include "conexionesFileSystem.h"
#include "mainFS.h"

void _enviarInfoBloque(int socket, DescriptorNodo * nodo, int bloque);

//a desarrollar
void responderYAMA(int socketYAMA){
	char rutaArchivo[255];
	recv(socketYAMA, rutaArchivo, sizeof(char) * 255, MSG_WAITALL);
	Archivo* descriptorArchivo = (Archivo*) dictionary_get(archivos, rutaArchivo);
	if(descriptorArchivo == NULL || !archivoDisponible(descriptorArchivo)){
		log_error(logger, "El archivo no se enontró o no está disponible\n");
		int mensaje = -2;
		send(socketYAMA, &mensaje, sizeof(mensaje), 0);
		return;
	}

	int mensaje = 0;
	int bloques = list_size(descriptorArchivo->bloques);

	send(socketYAMA, &mensaje, sizeof(mensaje), 0);
	send(socketYAMA, &bloques, sizeof(bloques), 0);

	int i;

	for(i = 0; i < bloques; ++i){
		Bloque * bloque = (Bloque *) list_get(descriptorArchivo->bloques, i);
		send(socketYAMA, &bloque->descriptor.bytes, sizeof(bloque->descriptor.bytes), 0);
		DescriptorNodo * descriptorNodoCopia0 = (DescriptorNodo *) dictionary_get(nodos, bloque->copia0.nodo);
		DescriptorNodo * descriptorNodoCopia1 = (DescriptorNodo *) dictionary_get(nodos, bloque->copia1.nodo);

		Ubicacion * ubicacion = NULL;
		DescriptorNodo * nodoCopia = NULL;

		if((descriptorNodoCopia0->socket == -1 || descriptorNodoCopia1->socket == -1) && bloque->otrasCopias != NULL){
			bool buscarCopia(void * ubicacion){
				Ubicacion * copia = (Ubicacion *)ubicacion;

				if(strcmp(copia->nodo, bloque->copia0.nodo) != 0 && strcmp(copia->nodo, bloque->copia1.nodo) != 0){
					nodoCopia = (DescriptorNodo *) dictionary_get(nodos, copia->nodo);
					return nodoCopia->socket != -1;
				}

				return false;
			}

			ubicacion = list_find(bloque->otrasCopias, buscarCopia);
		}

		if(descriptorNodoCopia0->socket == -1 || bloque->copia0.numeroBloque == -1){
			int cantidad = ubicacion == NULL ? 1 : 2;

			send(socketYAMA, &cantidad, sizeof(cantidad), 0);
			_enviarInfoBloque(socketYAMA, descriptorNodoCopia1, bloque->copia1.numeroBloque);

			if(ubicacion != NULL){
				_enviarInfoBloque(socketYAMA, nodoCopia, ubicacion->numeroBloque);
			}
		}else if(descriptorNodoCopia1->socket == -1 || bloque->copia1.numeroBloque == -1){
			int cantidad = ubicacion == NULL ? 1 : 2;
			send(socketYAMA, &cantidad, sizeof(cantidad), 0);
			_enviarInfoBloque(socketYAMA, descriptorNodoCopia0, bloque->copia0.numeroBloque);

			if(ubicacion != NULL){
				_enviarInfoBloque(socketYAMA, nodoCopia, ubicacion->numeroBloque);
			}
		}else{
			int cantidad = 2;
			send(socketYAMA, &cantidad, sizeof(cantidad), 0);
			_enviarInfoBloque(socketYAMA, descriptorNodoCopia0, bloque->copia0.numeroBloque);
			_enviarInfoBloque(socketYAMA, descriptorNodoCopia1, bloque->copia1.numeroBloque);
		}
	}
}

void _enviarInfoBloque(int socket, DescriptorNodo * nodo, int bloque){
	send(socket, nodo->nombreNodo, sizeof(char) * 100, 0);
	send(socket, nodo->ip, sizeof(char) * 20, 0);
	send(socket, &nodo->puerto, sizeof(nodo->puerto), 0);
	send(socket, &bloque, sizeof(bloque), 0);
}
