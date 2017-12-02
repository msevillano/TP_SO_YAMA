#include <commons/config.h>
#include <commons/log.h>
#include <stdint.h>

#ifndef WORKER_H_
#define WORKER_H_


/* ESTRUCTURAS */

typedef struct {
	int handshake;
	pid_t parentPid;
	pid_t childPid;
}Handshake;

typedef struct {
	char ip[20];
	uint16_t puerto;
} ConexionFileSystem;

typedef struct {
	int nombreLen;
	char nombreNodo[100];
	char rutaDataBin[255];
	uint16_t puertoWorker;
} __attribute__((packed))
InfoNodo;

typedef struct{
	long cantidadBytes;
	int bloque;
	char nombreTemp[255];
}mensajeTransf;

typedef struct{
	int cantidadTemporales;
	char archivoReducido[255];
} __attribute__((packed))
mensajeReduc;

/* VARIABLES GLOBALES Y DEFINES */
#define ARCHIVO_CONFIGURACION "worker.conf"
#define ARCHIVO_LOGGER "worker.log"
#define MODULO "WORKER"
#define IP_FILESYSTEM "IP_FILESYSTEM"
#define PUERTO_FILESYSTEM "PUERTO_FILESYSTEM"
#define NOMBRE_NODO "NOMBRE_NODO"
#define RUTA_DATABIN "RUTA_DATABIN"
#define PUERTO_WORKER "PUERTO_WORKER"
#define PUERTO_DATANODE "PUERTO_DATANODE"
#define MB 1048576

t_config * config;
t_log * logger;
ConexionFileSystem conexionFileSystem;
InfoNodo infoNodo;
char * mapeoDataBin;
int socketFS;

/* FUNCIONES */
void inicializarWorker();

#endif /* WORKER_H_ */
