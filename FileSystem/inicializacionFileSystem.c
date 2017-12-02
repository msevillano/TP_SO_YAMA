#include "inicializacionFileSystem.h"
#include "configuracionFileSystem.h"
#include "estructurasFileSystem.h"
#include "utilidadesFileSystem.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

int _cargarBitMap(DescriptorNodo * nodo);
int _recorrerArchivosDirectorio(char * nombre);
int _procesarArchivo(char * rutaDirectorio, char * nombre, int directorio);
void _procesarBloques(t_config * archivo, Archivo * descriptorArchivo);

void cargarTablaNodos(t_config * tablaNodos){
    char ** nombresNodos = config_get_array_value(tablaNodos, NODOS);

    while(*nombresNodos){
        DescriptorNodo * newNodo = (DescriptorNodo *)malloc(sizeof(*newNodo));
        newNodo->socket = -1; // indica que no está conectado
        pthread_mutex_init ( &newNodo->semaforo, NULL);
        strcpy(newNodo->nombreNodo, *nombresNodos);
        // FALTA IP Y PUERTO y bitmap
        char totalNodoKey[105];
        getKeyTotalBloquesNodo(totalNodoKey, *nombresNodos);

        newNodo->bloques = config_get_int_value(tablaNodos, totalNodoKey);

        char libreNodoKey[105];

        getKeyBloquesLibresNodo(libreNodoKey, *nombresNodos);

        newNodo->bloquesLibres = config_get_int_value(tablaNodos, libreNodoKey);

        if(_cargarBitMap(newNodo) == 0){
            log_debug(logger, "%s\nBloques:%d\nBloques Libres:%d\n", *nombresNodos, newNodo->bloques, newNodo->bloquesLibres);
            dictionary_remove_and_destroy(nodos, newNodo->nombreNodo, free); // evito duplicados
            dictionary_put(nodos, newNodo->nombreNodo, newNodo);

            list_add(nombreNodos, newNodo->nombreNodo);
        }else{
            log_info(logger, "%s fue descartado por no tener bitmap", *nombresNodos);
            free(newNodo);
        }

        nombresNodos++;
    }
}

int cargarTablaDirectorio(){
    FILE * archivoDirectorio = fopen(config_get_string_value(config, PATH_TABLA_DIRECTORIO), "rb");

    if(archivoDirectorio == NULL){
        return -1;
    }

    int i = 0;
    while(i < 100){ // recorro toda la tabla
        fread(&tabla_Directorios[i], sizeof(tabla_Directorios[i]), 1, archivoDirectorio);

        if(tabla_Directorios[i].nombre[0] == '\0') break; // salgo del for porque ya no hay nada en el archivo

        if(tabla_Directorios[i].id != i){ // hubo un directorio en el medio que se borró
            int index = tabla_Directorios[i].id;
            tabla_Directorios[index] = tabla_Directorios[i];
            tabla_Directorios[i].nombre[0] = '\0'; // limipio el nombre => libero la opción
            i = index; // continúo desde el index que modifique en la tabla de directorio
        }

        listaArchivosDirectorios[i] = list_create();
        log_debug(logger, "Directorio: %s\n Padre: %s\nID:%d\n", tabla_Directorios[i].nombre, tabla_Directorios[i].padre == -1 ? "root" : tabla_Directorios[tabla_Directorios[i].padre].nombre, tabla_Directorios[i].id);
        i++;
    }

    fclose(archivoDirectorio);

    return 0;
}

int cargarTablaArchivos(){
    char rutaDirectorioArchivos[255];
    strcpy(rutaDirectorioArchivos, config_get_string_value(config, PATH_DIR_ARCHIVOS));

    DIR * directorio = opendir(rutaDirectorioArchivos);

    if(directorio == NULL){
        log_info(logger, "No se pudo encontrar el directorio %s", rutaDirectorioArchivos);
        return -1;
    }

    struct dirent * entradaDirectorio;

    while((entradaDirectorio = readdir(directorio)) != NULL){
        // ignoro directorio actual y directorio padre
        if(!strcmp(entradaDirectorio->d_name, ".")|| !strcmp(entradaDirectorio->d_name, "..")) continue;

        if(entradaDirectorio->d_type == DT_DIR) { // es un directorio?
            int statusCode = _recorrerArchivosDirectorio(entradaDirectorio->d_name);

            if(statusCode != 0) return -1;
        }
    }

    return 0;
}

int _cargarBitMap(DescriptorNodo * nodo){
    char * archivo = (char *) malloc(sizeof(char) * 255);
    sprintf(archivo, config_get_string_value(config, PATH_BITMAPS), nodo->nombreNodo);

    FILE * f = fopen(archivo, "r");

    if(f == NULL){
        return 1;
    }

    char strBitMap[nodo->bloques + 1];

    fread(strBitMap, nodo->bloques, 1, f);

    fclose(f);

    crearBitMap(nodo);

    int i;

    for(i = 0; i < nodo->bloques; ++i){
        if(strBitMap[i] == '0'){
            bitarray_clean_bit(nodo->bitmap, i);
        }else{
            bitarray_set_bit(nodo->bitmap, i);
        }
    }

    free(archivo);
    return 0;
}

int _recorrerArchivosDirectorio(char * nombre){
    char rutaDirectorio[255];

    sprintf(rutaDirectorio, config_get_string_value(config, PATH_DIR_ARCHIVOS_FORMAT), nombre);

    DIR * directorio = opendir(rutaDirectorio);

    if(directorio == NULL){
        log_info(logger, "No se pudo encontrar el directorio %s\n", rutaDirectorio);
        return -1;
    }

    struct dirent * entradaDirectorio;

    while((entradaDirectorio = readdir(directorio)) != NULL){
        if(entradaDirectorio->d_type == DT_REG) { // es un archivo?
            int status = _procesarArchivo(rutaDirectorio, entradaDirectorio->d_name, atoi(nombre));

            if(status != 0){
                log_error(logger, "El archivo %s no pudo ser cargado, el File System está corrupto\n", entradaDirectorio->d_name);
            }
        }
    }

    return 0;
}

int _procesarArchivo(char * rutaDirectorio, char * nombre, int directorio){
    char rutaArchivo[255];
    sprintf(rutaArchivo, "%s/%s", rutaDirectorio, nombre);
    t_config * archivo = config_create(rutaArchivo);

    if(archivo == NULL){
        return -1;
    }

    if( !config_has_property(archivo, TAMANIO) ||
        !config_has_property(archivo, TIPO)){
        return -1;
    }

    Archivo * descriptorArchivo = (Archivo *)malloc(sizeof(*descriptorArchivo));

    descriptorArchivo->tamanio = config_get_long_value(archivo, TAMANIO);

    if(!strcmp(config_get_string_value(archivo, TIPO), ARCHIVOTEXTO)){
        descriptorArchivo->tipo = TEXTO;
    }else{
        descriptorArchivo->tipo = BINARIO;
    }

    descriptorArchivo->directorioPadre = directorio;

    _procesarBloques(archivo, descriptorArchivo);

    calcularRuta(*descriptorArchivo, nombre, descriptorArchivo->ruta);

    registrarArchivo(descriptorArchivo);

    log_debug(logger, "Tamaño: %d\nTipo: %d\nDirectorio: %d\nLa ruta es:%s",
             descriptorArchivo->tamanio,
             descriptorArchivo->tipo,
             descriptorArchivo->directorioPadre,
             descriptorArchivo->ruta);

    return 0;
}

void _procesarBloques(t_config * archivo, Archivo * descriptorArchivo){
    int i = 0;
    char bytesBloque[14],
         bloqueCopia0[15],
         bloqueCopia1[15],
         bloqueCopian[15];

    descriptorArchivo->bloques = list_create();

    sprintf(bytesBloque, BLOQUE_I_BYTES, i);

    while(config_has_property(archivo, bytesBloque)){
        Bloque * bloque = (Bloque *) malloc(sizeof(*bloque));

        bloque->descriptor.numeroBloque = i;
        bloque->descriptor.bytes = config_get_int_value(archivo, bytesBloque);

        sprintf(bloqueCopia0, BLOQUE_I_COPIA_0, i);
        sprintf(bloqueCopia1, BLOQUE_I_COPIA_1, i);

        char ** copia0 = config_get_array_value(archivo, bloqueCopia0);
        char ** copia1 = config_get_array_value(archivo, bloqueCopia1);

        bloque->copia0.numeroBloque = atoi(copia0[1]);
        strcpy(bloque->copia0.nodo, copia0[0]);

        bloque->copia1.numeroBloque = atoi(copia1[1]);
        strcpy(bloque->copia1.nodo, copia1[0]);

        int numCopia = 2;
        sprintf(bloqueCopian, BLOQUE_I_COPIA_N, i, numCopia);
        while(config_has_property(archivo, bloqueCopian)){
            char ** copiaN = config_get_array_value(archivo, bloqueCopian);

            if(bloque->otrasCopias == NULL){
                bloque->otrasCopias = list_create();
            }

            Ubicacion * ubicacion = (Ubicacion *)malloc(sizeof(*ubicacion));

            strcpy(ubicacion->nodo, copiaN[0]);
            ubicacion->numeroBloque= atoi(copiaN[1]);

            list_add(bloque->otrasCopias, ubicacion);

            numCopia++;
            sprintf(bloqueCopian, BLOQUE_I_COPIA_N, i, numCopia);
        }

        i++;
        sprintf(bytesBloque, BLOQUE_I_BYTES, i);

        list_add(descriptorArchivo->bloques, bloque);
    }
}