#include "fileSystem.h"
#include "envioBloques.h"
#ifndef TP_2017_2C_LANZALLAMAS_RECEPCIONBLOQUES_H
#define TP_2017_2C_LANZALLAMAS_RECEPCIONBLOQUES_H

int obtenerArchivo(char * rutaArchivoFS, char * rutaArchivoFinal);
int punteroNodo;

typedef struct{
    DescriptorNodo * nodo;
    int bloque;
    long bytes;
    char datos[MB];
} InfoRecepcion;

#endif //TP_2017_2C_LANZALLAMAS_RECEPCIONBLOQUES_H
