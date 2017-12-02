/*
 * EnvioArchivo.h
 *
 *  Created on: Sep 14, 2017
 *      Author: utnso
 */

#ifndef ENVIOARCHIVO_H_
#define ENVIOARCHIVO_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <archivos.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

int enviarArchivo(int kernel_fd, char* path);


#endif /* ENVIOARCHIVO_H_ */
