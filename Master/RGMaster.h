/*
 * RGMaster.h
 *
 *  Created on: 22/10/2017
 *      Author: utnso
 */

#ifndef RGMASTER_H_
#define RGMASTER_H_

void reduccionGlobal(int socket);

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

#endif /* RGMASTER_H_ */
