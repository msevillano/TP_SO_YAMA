/*
 * archivos.c
 *
 *  Created on: 16/9/2017
 *      Author: utnso
 */
#include "archivos.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

bool verificarExistenciaDeArchivo(char* path) {
	FILE * archivoConfig = fopen(path, "r");
	if (archivoConfig!=NULL){
		 fclose(archivoConfig);
		 return true;
	}
	return false;
}

void guardarArchivo(char * path, char * datos, int longitud){
    int archivo = creat(path, S_IRWXU | S_IRWXG | S_IRWXO);

	write(archivo, datos, sizeof(*datos) * longitud);
	fsync(archivo);
	close(archivo);
}
