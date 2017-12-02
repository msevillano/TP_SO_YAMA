#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dataNode.h"
#include "operacionesDataNode.h"

void _cargarConfiguracion();
void _copiarFileSystemIP();
void _copiarNombreNodo();
void _copiarRutaDataBin();
void _crearLogger();
void _logConfig();
void _calcularCantidadDeBloques();
void _mapearDataBin();

void inicializarDataNode(){
	_crearLogger();
	_cargarConfiguracion();
	_logConfig();
}

void _cargarConfiguracion(){
	config = config_create(ARCHIVO_CONFIGURACION);

	if(config == NULL){
		log_error(logger, "El archivo de configuración %s no pudo ser encontrado\n", ARCHIVO_CONFIGURACION);
		exit(-1);
	}

	if (!config_has_property(config, IP_FILESYSTEM) ||
		!config_has_property(config, PUERTO_FILESYSTEM) ||
		!config_has_property(config, NOMBRE_NODO) ||
		!config_has_property(config, PUERTO_WORKER) ||
		!config_has_property(config, RUTA_DATABIN)){
		log_error(logger, "El archivo de configuración %s no tiene los campos necesarios\n", ARCHIVO_CONFIGURACION);
		exit(-2);
	}

	conexionFileSystem.puerto = htons(config_get_int_value(config, PUERTO_FILESYSTEM));
	infoNodo.puertoWorker = htons(config_get_int_value(config, PUERTO_WORKER));
	_copiarFileSystemIP();
	_copiarNombreNodo();
	_copiarRutaDataBin();
	_calcularCantidadDeBloques();
	//_mapearDataBin();
}

void _calcularCantidadDeBloques(){
	int fd = open(infoNodo.rutaDataBin, O_RDONLY);
	struct stat stats;
	fstat(fd, &stats);
	infoNodo.cantidadBloques = stats.st_size / MB;

	log_info(logger, "La cantidad de bloques es: %d", infoNodo.cantidadBloques);
}

void _mapearDataBin(){
	int fd = open(infoNodo.rutaDataBin, O_RDWR);

	if(fd == -1){
		log_error(logger, "No se encontró el data.bin en la ruta %s\n", infoNodo.rutaDataBin);
		exit(1);
	}

	mapeoDataBin = mmap(NULL, infoNodo.cantidadBloques * MB,
						PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if(mapeoDataBin == MAP_FAILED){
		log_error(logger, "No se pudo mapear el data.bin en la ruta\n");
		exit(1);
	}
}

void _copiarFileSystemIP(){
	char * ip;

	ip = config_get_string_value(config, IP_FILESYSTEM);

	int i = 0;

	// Copio el valor caracter a caracter porque ip es un char[20] y la config devuelve char *
	while(*ip != '\0'){
		conexionFileSystem.ip[i] = *ip;
		ip++;
		i++;
	}
}

void _copiarNombreNodo(){
	char * nombre;

	nombre = config_get_string_value(config, NOMBRE_NODO);

	int i = 0;

	// Copio el valor caracter a caracter porque nombre es un char[20] y la config devuelve char *
	while(*nombre != '\0'){
		infoNodo.nombreNodo[i] = *nombre;
		nombre++;
		i++;
	}

	infoNodo.nombreLen = ++i; // le sumo 1 a i para contar el \0
}

void _copiarRutaDataBin(){
	char * ruta;

	ruta = config_get_string_value(config, RUTA_DATABIN);

	int i = 0;

	// Copio el valor caracter a caracter porque ip es un char[20] y la config devuelve char *
	while(*ruta != '\0'){
		infoNodo.rutaDataBin[i] = *ruta;
		ruta++;
		i++;
	}
}

void _crearLogger(){
	logger = log_create(ARCHIVO_LOGGER, MODULO, true, LOG_LEVEL_INFO);
}

void _logConfig(){
	log_debug(logger, "Config:\nIP_FILESYSTEM: %s\nPUERTO_FILESYSTEM: %d\nNOMBRE_NODO: %s\nPUERTO_WORKER:%d\nRUTA_DATABIN: %s\n",
			  config_get_string_value(config, IP_FILESYSTEM),
			  config_get_int_value(config, PUERTO_FILESYSTEM),
			  config_get_string_value(config, NOMBRE_NODO),
			  config_get_int_value(config, PUERTO_WORKER),
			  config_get_string_value(config, RUTA_DATABIN));
}