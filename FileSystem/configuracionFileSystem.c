#include "configuracionFileSystem.h"
#include "estructurasFileSystem.h"
#include "fileSystem.h"
#include "string.h"
#include "stdio.h"

void _concatenarStringAlNombre(char * key, char * nombre, char * string);

void getKeyTotalBloquesNodo(char * key, char * nombreNodo){
    _concatenarStringAlNombre(key, nombreNodo, TOTAL);
}

void getKeyBloquesLibresNodo(char * key, char * nombreNodo){
    _concatenarStringAlNombre(key, nombreNodo, Libre);
}

void _concatenarStringAlNombre(char * key, char * nombre, char * string){
    strcpy(key, nombre);
    strcat(key, string);
}

void intToString(int numero, char * string){
    sprintf(string, "%d", numero);
}

void longToString(long numero, char * string){
    sprintf(string, "%ld", numero);
}