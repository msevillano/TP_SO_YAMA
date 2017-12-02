#include "enviarArchivo.h"
#include "master.h"
#include <archivos.h>
#include <sockets.h>

int enviarArchivo(int socket, char* path){

	//Verifico existencia archivo (Aguante esta funcion loco!)
	if( !verificarExistenciaDeArchivo(path) ){
		return -1;
	}

	FILE* sourceFile;
	int file_fd, file_size;
	struct stat stats;

	//Abro el archivo y le saco los stats
	sourceFile = fopen(path, "r");
	//esto nunca deberia fallar porque ya esta verificado, pero por las dudas
	if(sourceFile == NULL){
		log_error(logger, "No pudo abrir el archivo");
		return -1;
	}
	file_fd = fileno(sourceFile);

	fstat(file_fd, &stats);
	file_size = stats.st_size;
	char* buffer = malloc(file_size + 1);
	int offset = 0;

	if(buffer == NULL){
		log_error(logger, "No se pudo reservar memoria para enviar archivo");
		fclose(sourceFile);
		return -1;
	}


	if(fread(buffer + offset,file_size,1,sourceFile) < 1){
		log_error(logger, "No es posible leer el archivo");
		free(buffer);
		fclose(sourceFile);
		return -1;
	}


	/*Esto lo hago asi porque de la otra forma habría que reservar MAS espacio para
     * enviar el paquete */
	if(sendAll(socket, buffer, file_size, 0) <= 0){
		log_error(logger, "Error al enviar archivo");
		free(buffer);
		fclose(sourceFile);
		return -1;
	}
	free(buffer);
	fclose(sourceFile);
	return 0;
}