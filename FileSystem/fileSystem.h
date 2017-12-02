#include <stdbool.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <pthread.h>

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

/* ENUMS */
typedef enum{TEXTO=0, BINARIO} TipoArchivo;

/* ESTRUCTURAS */
typedef struct t_Directorio{
    int id;
    char nombre[255];
    int padre;
} Directorio;

typedef struct t_DescriptorNodo{
    char nombreNodo[100];
    char ip[20];
    uint16_t puerto;
    int bloques;
    t_bitarray * bitmap;
    int socket;
    int bloquesLibres;
    pthread_mutex_t semaforo;
} DescriptorNodo;

// Falta el bitmap, no se cuál es la mejor
typedef struct t_DataNode{
    DescriptorNodo descriptor;
    int bloquesTotales;
    int bloquesLibres;
} DataNode;

typedef struct t_Ubicacion{
    char nodo[100];
    int numeroBloque;
} Ubicacion;

typedef struct t_DescriptorBloque{
    int numeroBloque;
    long bytes;
} DescriptorBloque;

typedef struct t_Bloque{
    DescriptorBloque descriptor;
    Ubicacion copia0;
    Ubicacion copia1;
    t_list * otrasCopias;
} Bloque;

typedef struct t_Archivo{
    TipoArchivo tipo;
    t_list * bloques;
    char ruta[255];
    long tamanio;
    int directorioPadre;
} Archivo;

Directorio tabla_Directorios [100];
t_list* listaArchivosDirectorios[100]; // 100 listas una por directorio para poner punteros a sus archivos

/* VARIABLES GLOBALES y DEFINES*/
#define ARCHIVO_LOGGER "fileSystem.log"
#define ARCHIVO_CONFIGURACION "fileSystem.conf"
#define MODULO "FILESYSTEM"
#define MAX_LENGTH 255
#define MAX_DIRECTORIOS 100
#define PUERTO_FILESYSTEM "PUERTO_FILESYSTEM"
#define PATH_TABLA_NODOS "PATH_TABLA_NODOS"
#define PATH_BITMAPS "PATH_BITMAPS"
#define PATH_TABLA_DIRECTORIO "PATH_TABLA_DIRECTORIO"
#define PATH_DIR_ARCHIVOS "PATH_DIR_ARCHIVOS"
#define PATH_DIR_ARCHIVOS_FORMAT "PATH_DIR_ARCHIVOS_FORMAT"
#define ROOT_DIR "root"

t_dictionary* nodos;
t_list * nombreNodos;
t_dictionary* archivos;
t_log * logger;
t_config * config;
extern pthread_mutex_t semaforoConsola;
int fileSystemEstable;
fd_set master;   // conjunto maestro de descriptores de fichero
int hayData;
DescriptorNodo * nodoConectado;

/* FUNCIONES */
void asociarNodo(int socket);
void inicializarFileSystem();
void persistirFileSystem();
void crearRootDir();

#endif /* FILESYSTEM_H_ */
