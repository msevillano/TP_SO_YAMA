#include <commons/config.h>
#include <commons/log.h>
#include <stdint.h>

#ifndef MASTER_H_
#define MASTER_H_

/* ESTRUCTURAS */

typedef struct {
	pid_t parentPid;
	pid_t childPid;
}Handshake;

typedef struct{
	int transformacion;
	int cantidadWorkers;
}Transformacion;

typedef struct {
	char ip[20];
	uint16_t puerto;
} YamaConexion;

typedef struct {
	int tipoMensaje;
	char rutaArchivo[255];
} SolicitudJob;

typedef struct {
	char nombreNodo[100];
	char ipWorker[20];
	uint16_t puertoWorker;
	int numBloque;
	int bytesOcupados;
	char rutaArchivo[255];
} __attribute__((packed))
workerTransformacion;

typedef struct{
	char nombreNodo[100];
	char ip[20];
	uint16_t puerto;
	int cantidadTemporales;
	char archivoReducido[255];
	char temporales[100][255];
} __attribute__((packed))
OperacionReduccion;

typedef struct{
	char nombreNodo[100];
	char ip[20];
	uint16_t puerto;
	char archivoReducido[255];
} __attribute__((packed))
OperacionReduccionGlobal;

typedef struct{
	int cantidadTemporales;
	char archivoReducido[255];
} __attribute__((packed))
reduccionWorker;

typedef struct{
	char ruta[255];
}rutaArchivo;

/* VARIABLES GLOBALES Y DEFINES */
#define ARCHIVOCFG "master.conf"
#define ARCHIVO_LOGGER "master.log"
#define MODULO "MASTER"
#define YAMA_IP "YAMA_IP"
#define YAMA_PUERTO "YAMA_PUERTO"


YamaConexion conexionYAMA;
t_log * logger;
t_config * config;
int jobId;
char * transformador;
char * reductor;
char * pathArchivoFS;


/* FUNCIONES */
void inicializarMaster();


#endif /* MASTER_H_ */
