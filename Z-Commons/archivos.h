/*
 * archivos.h
 *
 *  Created on: 16/9/2017
 *      Author: utnso
 */
#include <stdbool.h>

#ifndef ARCHIVOS_H_
#define ARCHIVOS_H_

bool verificarExistenciaDeArchivo(char* path);
void guardarArchivo(char * path, char * datos, int longitud);

#endif /* ARCHIVOS_H_ */
