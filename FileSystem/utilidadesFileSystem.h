#include "fileSystem.h"

#ifndef TP_2017_2C_LANZALLAMAS_UTILIDADESFILESYSTEM_H
#define TP_2017_2C_LANZALLAMAS_UTILIDADESFILESYSTEM_H

void calcularRuta(Archivo descriptorArchivo, char * nombreArchivo, char * rutaArchivo);
void registrarArchivo(Archivo * descriptorArchivo);
char * obtenerNombreArchivo(char * ruta);
void obtenerNuevaRutaArchivo(char * rutaVieja, char * rutaDir);
int calcularDirectorioPadre(char * ruta);
int calcularEntradaDirectorio(char * dir);
int archivoDisponible(Archivo * archivo);
t_list * obtenerNombresDirectoriosHijos(int dir);
int obtenerIdDirectorio();
int cantidadDirectoriosHijos(int dir);
void destruirArchivo(Archivo * archivo);
void liberarBloque(Ubicacion bloque);
bool nombreValido(int dir, char * nuevoNombre);
void calcularRutaDirectorio(int dir, char * ruta);
void _agregarAlPrincipio(char* stringOriginal, char* stringAAgregarAlPrincipio);
#endif //TP_2017_2C_LANZALLAMAS_UTILIDADESFILESYSTEM_H
