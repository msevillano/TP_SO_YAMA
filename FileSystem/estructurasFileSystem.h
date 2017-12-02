#include "fileSystem.h"

#ifndef TP_2017_2C_LANZALLAMAS_ESTRUCTURASFILESYSTEM_H
#define TP_2017_2C_LANZALLAMAS_ESTRUCTURASFILESYSTEM_H

#define TAMANIO "TAMANIO"
#define LIBRE "LIBRE"
#define Libre "Libre"
#define TOTAL "Total"
#define NODOS "NODOS"

void guardarFileSystem();
void guardarTablaNodos();
void agregarNodoEnTabla(DescriptorNodo * newNodo);
void crearBitMap(DescriptorNodo * newNodo);
void guardarTablaDirectorio();
void guardarArchivos();
void marcarNodoDesconectado(int socket);

#endif //TP_2017_2C_LANZALLAMAS_ESTRUCTURASFILESYSTEM_H
