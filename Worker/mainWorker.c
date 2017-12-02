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
#include "chat.h"
#include "worker.h"
#include "servidor.h"
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "cliente.h"

void inicializarWorker();
void _cargarConfiguracion();
void _mapearDataBin();
void _crearLogger();

void nuevoCliente(char* remoteHost, int newfd){
	printf("new conection from %s on socket %d\n", remoteHost, newfd);
	//Y acá hacer algo con el nuevo cliente conectado
}


int main(int argc, char *argv[]) {
	inicializarWorker();


	setServer();

	for(;;);

	return 0;
}

void inicializarWorker(){
	_crearLogger();
	_cargarConfiguracion();
	iniciarConexionAServer(&socketFS);
}

void _crearLogger(){
	logger = log_create(ARCHIVO_LOGGER, MODULO, true, LOG_LEVEL_INFO);
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

	_mapearDataBin();
}

void _mapearDataBin(){
	int fd = open(config_get_string_value(config, RUTA_DATABIN), O_RDONLY);

	if(fd == -1){
		log_error(logger, "No se encontró el data.bin en la ruta %s\n", infoNodo.rutaDataBin);
		exit(1);
	}

	struct stat stats;
	fstat(fd, &stats);

	mapeoDataBin = mmap(NULL, stats.st_size,
						PROT_READ, MAP_PRIVATE, fd, 0);

	if(mapeoDataBin == MAP_FAILED){
		log_error(logger, "No se pudo mapear el data.bin en la ruta\n");
		exit(1);
	}
}