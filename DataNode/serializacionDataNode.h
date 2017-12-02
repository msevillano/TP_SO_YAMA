/*
 * serializacion.h
 *
 *  Created on: 12/9/2017
 *      Author: utnso
 */

#ifndef SERIALIZACIONDATANODE_H_
#define SERIALIZACIONDATANODE_H_

typedef enum {DATANODEERROR=0, DATANODEOK} CODIGOSFILESYSTEM;

void manejarDatos(int buf, int socket);

#endif /* SERIALIZACIONDATANODE_H_ */
