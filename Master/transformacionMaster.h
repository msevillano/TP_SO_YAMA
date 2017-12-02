#ifndef TRANSFORMACIONMASTER_H_
#define TRANSFORMACIONMASTER_H_
#include <commons/collections/list.h>

typedef struct{
	long cantidadBytes;
	int bloque;
	char nombreTemp[255];
}__attribute__((packed))
mensajeTransformacion;

typedef struct{
	char nombreNodo[100];
	int numBloque;
	int bytes;
	char nombreTemp[255];
}__attribute__((packed))
transfError;

int respuestaSolicitud();
void mandarTransformacionNodo();
void atenderConexion(int socket);

#endif /* TRANSFORMACIONMASTER_H_ */
