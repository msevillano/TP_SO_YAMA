/*
 * dataNode.h
 *
 *  Created on: 10/9/2017
 *      Author: utnso
 */

#include <commons/config.h>
#include <commons/log.h>
#include <stdint.h>
#include <sys/mman.h>

#ifndef DATANODE_H_
#define DATANODE_H_

/* ESTRUCTURAS */

typedef struct {
	char ip[20];
	uint16_t puerto;
} ConexionFileSystem;

typedef struct t_handshake{
	int tipomensaje;
	char nombreNodo[100];
	char ip[20];
	uint16_t puerto;
	int bloques;
} __attribute__((packed))
		HandshakeNodo;

typedef struct {
	int nombreLen;
	char nombreNodo[100];
	char rutaDataBin[255];
	uint16_t puertoWorker;
	int cantidadBloques;
} __attribute__((packed))
		InfoNodo;

/* VARIABLES GLOBALES Y DEFINES */
#define ARCHIVO_CONFIGURACION "dataNode.conf"
#define ARCHIVO_LOGGER "dataNode.log"
#define MODULO "DATANODE"
#define IP_FILESYSTEM "IP_FILESYSTEM"
#define PUERTO_FILESYSTEM "PUERTO_FILESYSTEM"
#define NOMBRE_NODO "NOMBRE_NODO"
#define PUERTO_WORKER "PUERTO_WORKER"
#define RUTA_DATABIN "RUTA_DATABIN"

t_config * config;
t_log * logger;
ConexionFileSystem conexionFileSystem;
InfoNodo infoNodo;
char * mapeoDataBin;

/* FUNCIONES */
void inicializarDataNode();

#endif /* DATANODE_H_ */