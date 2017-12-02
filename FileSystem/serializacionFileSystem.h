/*
 * serializacion.h
 *
 *  Created on: 4/9/2017
 *      Author: utnso
 */

#ifndef SERIALIZACIONFILESYSTEM_H_
#define SERIALIZACIONFILESYSTEM_H_

enum tipoProceso {YAMA, FILESYSTEM, DATANODE, MASTER, WORKER};

void manejarDatos(int buf, int socket);

#endif /* SERIALIZACIONFILESYSTEM_H_ */
