#ifndef RGWORKER_H_
#define RGWORKER_H_

#include <stdint.h>

void iniciarGlobal(int socket_master);
void rutinaNoEncargado(int socket);

typedef struct{
	char nombre[100];
	char ip[20];
	uint16_t puerto;
	char archivoReducido[255];
}__attribute__((packed))
NodoGlobal;

typedef struct{
	char nombre[100];
	char ip[20];
	uint16_t puerto;
	char archTemp[255];
}__attribute__((packed))
NodoEncargado;

#endif /* RGWORKER_H_ */