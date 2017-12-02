#include "estructurasFileSystem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/string.h>
#include "fileSystem.h"
#include "utilidadesFileSystem.h"
#include "inicializacionFileSystem.h"
#include "configuracionFileSystem.h"
#include <sys/types.h>
#include <sys/stat.h>

void _persistirNodo(char * key, void * value);
void _persistirBitMap(char * strBitMap, char * nombreNodo, int bloques);
void _persistirArchivo(char * key, void * value);
void _persistirBloque(void * puntero);
void _marcarNodoDesconectadoImpl(char * key, void * value);
int _contieneNombreNodo(char * nombreNodo);

// Declaro estas variables globales para acumular durante la iteracion del diccionario (guardado de nodos)
int bloquesTotales;
int bloquesLibres;
t_config * tablaNodos;
char * nombres;

// Declaro estas variables globales para acumular durante la iteración de la lista (guardado de archivos)
t_config * archivoConfig;

// Declaro el socket para pasarle al iterator
int socketNodo;

void guardarFileSystem(){
    guardarTablaNodos();
    guardarTablaDirectorio();
    guardarArchivos();
}

void agregarNodoEnTabla(DescriptorNodo * newNodo){
    DescriptorNodo * descriptorNodo = (DescriptorNodo *) dictionary_get(nodos, newNodo->nombreNodo);

    if(descriptorNodo == NULL){
        crearBitMap(newNodo);

        dictionary_remove_and_destroy(nodos, newNodo->nombreNodo, free); // evito duplicados
        dictionary_put(nodos, newNodo->nombreNodo, newNodo);

        list_add(nombreNodos, newNodo->nombreNodo);
    }else {
        descriptorNodo->socket = newNodo->socket;
        strcpy(descriptorNodo->ip, newNodo->ip);
        descriptorNodo->puerto = newNodo->puerto;

        dictionary_remove(nodos, descriptorNodo->nombreNodo);
        dictionary_put(nodos, descriptorNodo->nombreNodo, descriptorNodo);
    }
}

void crearBitMap(DescriptorNodo * newNodo){
    // divido la cantidad de bloques por 8 para tener un bit por bloque; el + 7 permite que: 2 / 8 = 1
    int bytes = (newNodo->bloques + 7) / 8;

    char * bitArray = (char *) malloc(sizeof(char) * bytes);
    memset(bitArray, 0, sizeof(char) * bytes);

    newNodo->bitmap = bitarray_create_with_mode(bitArray, bytes, LSB_FIRST);
}

void guardarTablaNodos(){
    char * archivo = config_get_string_value(config, PATH_TABLA_NODOS);

    int cantidadNodos = dictionary_size(nodos);

    if(cantidadNodos == 0){
        return;
    }

    FILE * archivoNodos = fopen(archivo, "w"); // hago esto para crear el archivo si no existe
    fclose(archivoNodos);

    /*
     * 100 para el nombre de cada nodo
     * 1 coma por cada nodo
     * 2 corchetes
     */
    int chars = 101 * cantidadNodos + 2;
    int bytes = sizeof(char) * chars;
    nombres = (char *) malloc(bytes);
    memset(nombres, 0, bytes);

    tablaNodos = config_create(archivo);
    bloquesTotales = 0;
    bloquesLibres = 0;
    dictionary_iterator(nodos,_persistirNodo);

    char strBloquesTotales[5];
    intToString(bloquesTotales, strBloquesTotales);

    config_set_value(tablaNodos, TAMANIO, strBloquesTotales);

    char strBloquesLibres[5];
    intToString(bloquesLibres, strBloquesLibres);

    config_set_value(tablaNodos, LIBRE, strBloquesLibres);

    strcat(nombres, "]");

    config_set_value(tablaNodos, NODOS, nombres);

    config_save_in_file(tablaNodos, archivo);
    config_destroy(tablaNodos);
    free(nombres);
}

void guardarTablaDirectorio(){
    FILE * archivoDirectorio = fopen(config_get_string_value(config, PATH_TABLA_DIRECTORIO), "wb");

    int i;

    for(i = 0; i < 100; ++i){ // recorro toda la tabla
        // directorio no existente lo saleo, continuo el for porque puede haber otros si el directorio fue borrado
        if(tabla_Directorios[i].nombre[0] == '\0') continue;

        fwrite(&tabla_Directorios[i], sizeof(tabla_Directorios[i]), 1, archivoDirectorio);
    }

    fclose(archivoDirectorio);
}

void guardarArchivos(){
    dictionary_iterator(archivos, _persistirArchivo);
}

void marcarNodoDesconectado(int socket){
    socketNodo = socket;
    dictionary_iterator(nodos,_marcarNodoDesconectadoImpl);
}

void _marcarNodoDesconectadoImpl(char * key, void * value){
    DescriptorNodo * descriptorNodo = (DescriptorNodo *) value;
    if(descriptorNodo->socket == socketNodo){
        descriptorNodo->socket = -1;
        log_info(logger, "Se desconectó el nodo: %s\n", key);
    }
}

void _persistirNodo(char * key, void * value){
    DescriptorNodo * nodo = (DescriptorNodo *)value;

    if(bloquesTotales){
        string_append_with_format(&nombres, ",%s", nodo->nombreNodo);
    }else{
        string_append_with_format(&nombres, "[%s", nodo->nombreNodo);
    }

    bloquesTotales += nodo->bloques;

    int bitmapSize = nodo->bloques;
    char strBitMap[bitmapSize + 1];
    memset(strBitMap, 0, bitmapSize + 1);

    while(bitmapSize--){
        if(bitarray_test_bit(nodo->bitmap, bitmapSize) == 0){ // bloque libre
            strBitMap[bitmapSize] = '0';
        }else{
            strBitMap[bitmapSize] = '1';
        }
    }

    bloquesLibres += nodo->bloquesLibres;

    char totalNodoKey[105];

    getKeyTotalBloquesNodo(totalNodoKey, nodo->nombreNodo);

    char strBloquesNodo[5];
    intToString(nodo->bloques, strBloquesNodo);

    config_set_value(tablaNodos, totalNodoKey, strBloquesNodo);

    char libreNodoKey[105];

    getKeyBloquesLibresNodo(libreNodoKey, nodo->nombreNodo);

    char strBloquesLibresNodo[5];
    intToString(nodo->bloquesLibres, strBloquesLibresNodo);

    config_set_value(tablaNodos, libreNodoKey, strBloquesLibresNodo);

    _persistirBitMap(strBitMap, nodo->nombreNodo, nodo->bloques);
}

void _persistirBitMap(char * strBitMap, char * nombreNodo, int bloques){
    char * archivo = (char *) malloc(sizeof(char) * 255);
    sprintf(archivo, config_get_string_value(config, PATH_BITMAPS), nombreNodo);

    FILE * f = fopen(archivo, "w");

    fwrite(strBitMap, bloques, 1, f);
    fclose(f);
    free(archivo);
}

void _persistirArchivo(char * key, void * value){
    char rutaArchivo[255],
         strDirectorioPadre[3];
    Archivo * descriptorArchivo = (Archivo *) value;

    intToString(descriptorArchivo->directorioPadre, strDirectorioPadre);
    sprintf(rutaArchivo, config_get_string_value(config, PATH_DIR_ARCHIVOS_FORMAT), strDirectorioPadre);
    strcat(rutaArchivo, "/");

    mkdir(rutaArchivo, S_IRWXU | S_IRWXG | S_IRWXO); // creo el directorio si no existe

    strcat(rutaArchivo, obtenerNombreArchivo(descriptorArchivo->ruta));

    FILE * archivoNodos = fopen(rutaArchivo, "w"); // hago esto para crear el archivo si no existe
    fclose(archivoNodos);

    archivoConfig = config_create(rutaArchivo);

    char strTamanio[15];
    intToString(descriptorArchivo->tamanio, strTamanio);

    config_set_value(archivoConfig, TAMANIO, strTamanio);
    config_set_value(archivoConfig, TIPO, descriptorArchivo->tipo ? ARCHIVOBINARIO : ARCHIVOTEXTO);

    list_iterate(descriptorArchivo->bloques, _persistirBloque);

    config_save_in_file(archivoConfig, rutaArchivo);
    config_destroy(archivoConfig);
}

void _persistirBloque(void * puntero){
    Bloque * bloque = (Bloque *)puntero;

    char bytesBloque[14],
         bloqueCopia0[15],
         bloqueCopia1[15],
         strCopia0[110],
         strCopia1[110],
         strBytes[15];

    sprintf(bytesBloque, BLOQUE_I_BYTES, bloque->descriptor.numeroBloque);
    sprintf(bloqueCopia0, BLOQUE_I_COPIA_0, bloque->descriptor.numeroBloque);
    sprintf(bloqueCopia1, BLOQUE_I_COPIA_1, bloque->descriptor.numeroBloque);

    sprintf(strCopia0, "[%s, %d]", bloque->copia0.nodo, bloque->copia0.numeroBloque);
    sprintf(strCopia1, "[%s, %d]", bloque->copia1.nodo, bloque->copia1.numeroBloque);
    longToString(bloque->descriptor.bytes, strBytes);

    config_set_value(archivoConfig, bloqueCopia0, strCopia0);
    config_set_value(archivoConfig, bloqueCopia1, strCopia1);
    config_set_value(archivoConfig, bytesBloque, strBytes);

    if(bloque->otrasCopias != NULL){
        int i = 2;
        void iterator(void * ubicacion){
            Ubicacion * copia = (Ubicacion *) ubicacion;
            char bloqueCopian[15],
            strCopian[110];

            sprintf(bloqueCopian, BLOQUE_I_COPIA_N, bloque->descriptor.numeroBloque, i++);
            sprintf(strCopian, "[%s, %d]", copia->nodo, copia->numeroBloque);

            config_set_value(archivoConfig, bloqueCopian, strCopian);
        }

        list_iterate(bloque->otrasCopias, iterator);
    }
}

int _contieneNombreNodo(char * nombreNodo){
    int longitud = list_size(nombreNodos);
    int i = 0;

    for(i = 0; i < longitud; ++i){
        if(strcmp(nombreNodo, list_get(nombreNodos, i))){
            return 0;
        }
    }

    return 1;
}