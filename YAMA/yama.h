

#ifndef YAMA_H_
#define YAMA_H_

#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <stdint.h>
#include "servidor.h"

/* ENUMS */
typedef enum{TRANSFORMACION=1, REDUCCIONLOCAL, REDUCGLOBAL, ALMACENAMIENTOOP} TipoOperacion;
typedef enum{ENPROCESO=1, ERRORYAMA, FINALIZADO} Estado;
typedef enum{CLOCK=1, WEIGHTEDCLOCK} AlgoritmoBalanceo;

/* ESTRUCTURAS */

typedef struct{
	int tipomensaje;
	char ruta[255];
} __attribute__((packed)) SolicitudFS;

typedef struct t_InfoNodo{
	char nombre[100];
	char ip[20];
	uint16_t puerto;
	long bytes;
	int disponibilidad;
	int trabajosActuales;
	t_dictionary * bloques;
} InfoNodo;

typedef struct t_TablaEstado{
	int jobId;
	int masterId;
	char nombreNodo[100];
	char ip[20];
	int disponibilidad;
	int trabajoActual;
	int historicas;
	uint16_t puerto;
	int numeroBloque;
	int tamBloque;
	TipoOperacion etapa;
	char archivoTemporal[255];
	Estado estado;
	InfoNodo * nodoCopia;
	char bloqueArchivo[5];
} __attribute__((packed)) EntradaTablaEstado;

typedef struct{
	char ruta[255];
}rutaArchivo;

typedef struct{
	char nombreNodo[100];
	char ip[20];
	uint16_t puerto;
	int cantidadTemporales;
	char archivoReducido[255];
} __attribute__((packed)) operacionReduccion;

typedef struct{
	char nombreNodo[100];
	char ip[20];
	uint16_t puerto;
	int bloque;
	int bytes;
	char ruta[255];
} __attribute__((packed)) operacionTransformacion;

// Está repetido sacar de acá cuando hagamos una librería común
typedef struct t_DescriptorBloque{
	int numeroBloque;
	long bytes;
	char copia0;
	char copia1;
	char nodoAsignado;
	struct t_DescriptorBloque *siguiente;
}DescriptorBloque;

//typedef struct t_DescriptorBloque DescriptorBloque;

typedef struct {
	char ip[20];
	uint16_t puerto;
} ConexionFileSystem;



typedef struct {
	char nombreNodo[100];
	char ip[20];
	uint16_t puerto;
	int bloque;
} DescriptorNodo;

typedef struct {
	int bloque;
	long bytes;
} TamanoBloque;

typedef struct{
	char nombre[100];
	char ip[20];
	uint16_t puerto;
	char archTemp[255];
}__attribute__((packed))
NodoGlobal;

typedef struct{
	char nombre[100];
	char ip[20];
	uint16_t puerto;
	char archTemp[255];
}__attribute__((packed))
NodoEncargado;

typedef struct{
	int jobId;
	int socket;
} SocketJob;

/* VARIABLES GLOBALES Y DEFINES*/
#define ARCHIVO_CONFIGURACION "yama.conf"
#define ARCHIVO_LOGGER "yama.log"
#define MODULO "YAMA"
#define FS_IP "FS_IP"
#define FS_PUERTO "FS_PUERTO"
#define RETARDO_PLANIFICACION "RETARDO_PLANIFICACION"
#define ALGORITMO_BALANCEO "ALGORITMO_BALANCEO"
#define CLOCK_CONFIG "CLOCK"
#define WEIGHTED_CLOCK "WEIGHTED_CLOCK"
#define DISPONIBILIDAD_BASE "DISPONIBILIDAD_BASE"

t_config * config;
t_log * logger;
ConexionFileSystem conexionFileSystem;
AlgoritmoBalanceo algoritmoBalanceo;
int retardoPlanificacion;
int sock_fs;
t_list* tablaEstado;
t_list* socketJobs;
int disponibilidad_base;
int cantidadJobs;
int cabecera;

/* FUNCIONES */
void inicializarYAMA();
void recargarConfiguracion();
void intToString(int numero, char * string);

#endif /* YAMA_H_ */

