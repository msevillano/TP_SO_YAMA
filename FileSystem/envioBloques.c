#include "fileSystem.h"
#include "envioBloques.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "conexionesFileSystem.h"
#include "utilidadesFileSystem.h"
#include <sockets.h>

int _controlarEspacioDisponible(int cantidadBloques);
void _contarBloquesLibres(char * key, void * nodo);
int _enviarBloque(char * bloque, Ubicacion ubicacion);
Bloque * _crearBloque(int numeroBloque, long bytes);
void _calcularUbicacionBloque(Bloque * bloque);
int _obtenerNumeroBloqueLibre(t_bitarray * bitmap);
int _manejarEnvioBloque(char * bloque, Bloque * descriptorBloque);
int _obtenerData(int socket, int bloque,  char * contenidoBloque);
int _enviarArchivoTexto(FILE * file, Archivo * descriptorArchivo);
int _enviarArchivoBinario(FILE * file, Archivo * descriptorArchivo, int cantidadBloques);

int bloquesLibres = 0;

int enviarBloques(char * archivo, char * archivoYamaFS, int binario){
    FILE * file = fopen(archivo, "r");

    if(file == NULL){ // no se encontró el archivo
        log_error(logger, "El archivo %s no existe\n", archivo);
        return -1;
    }

    int directorioPadre = calcularDirectorioPadre(archivoYamaFS);

    if(directorioPadre < 0){
        log_error(logger, "La ruta %s es inválida\n", archivoYamaFS);
        return -1;
    }

    struct stat fileStats;

    if(stat(archivo, &fileStats) != 0){ // error
        log_error(logger, "El archivo %s no existe\n", archivo);
        return -1;
    }

    int cantidadBloques = fileStats.st_size / MB + 1;

    if(!_controlarEspacioDisponible(cantidadBloques)){ // no hay espacio
        log_error(logger, "No hay espacio disponible para guardar el archivo; El archivo requiere %d bloques para cada copia; %d archivos en total\n", cantidadBloques, 2 * cantidadBloques);
        return -1;
    }

    Archivo * descriptorArchivo = (Archivo *)malloc(sizeof(*descriptorArchivo));
    strcpy(descriptorArchivo->ruta, archivoYamaFS);
    descriptorArchivo->tipo = binario ? BINARIO : TEXTO;
    descriptorArchivo->tamanio = fileStats.st_size;
    descriptorArchivo->directorioPadre = directorioPadre;
    descriptorArchivo->bloques = list_create();

    int status;
    if(binario){
        status = _enviarArchivoBinario(file, descriptorArchivo, cantidadBloques);
    }else{
        status = _enviarArchivoTexto(file, descriptorArchivo);
    }

    fclose(file);

    if(status != 0){
        return status;
    }

    registrarArchivo(descriptorArchivo);

    return 0;
}

int _enviarArchivoBinario(FILE * file, Archivo * descriptorArchivo, int cantidadBloques){
    char bloque[MB];
    int i;

    for(i = 0; i < cantidadBloques; ++i){
        memset(bloque, 0, MB); // limpio el bloque

        fread(bloque, sizeof(char) * MB, 1, file);

        Bloque * descriptorBloque = _crearBloque(i, strlen(bloque));

        if(_manejarEnvioBloque(bloque, descriptorBloque) == -1){
            destruirArchivo(descriptorArchivo);
            log_error(logger, "Error guardando los bloques en los datanode\n");
            return -1;
        }

        list_add(descriptorArchivo->bloques, descriptorBloque);
    }

    return 0;
}

int _enviarArchivoTexto(FILE * file, Archivo * descriptorArchivo){
    char bloque[MB];

    memset(bloque, 0, MB); // limpio el bloque
    long ocupados = 0;

    ssize_t bytesLeidos;
    char * linea = NULL;
    size_t len;
    int numeroBloque = 0;

    while((bytesLeidos = getline(&linea, &len, file)) > -1) { // hasta llegar al final del archivo
        if (ocupados + bytesLeidos > MB) { // no alcanza el bloque para guardar el renglón
            Bloque * descriptorBloque = _crearBloque(numeroBloque, ocupados);

            ocupados = 0;

            if(_manejarEnvioBloque(bloque, descriptorBloque) == -1){
                destruirArchivo(descriptorArchivo);
                log_error(logger, "Error guardando los bloques en los datanode\n");
                return -1;
            }

            list_add(descriptorArchivo->bloques, descriptorBloque);
            memset(bloque, 0, MB);
            numeroBloque++;
        }

        strcat(bloque + ocupados, linea);
        ocupados += bytesLeidos;
    }

    if(ocupados != 0){ // quedan bytes para mandar para mandar
        Bloque * descriptorBloque = _crearBloque(numeroBloque, ocupados);

        if(_manejarEnvioBloque(bloque, descriptorBloque) == -1){
            destruirArchivo(descriptorArchivo);
            log_error(logger, "Error guardando los bloques en los datanode\n");
            return -1;
        }

        list_add(descriptorArchivo->bloques, descriptorBloque);
    }

    return 0;
}


int generarCopia(Bloque * bloque, DescriptorNodo * descriptorNodo){
    if(descriptorNodo->bloquesLibres == 0){
        return -1;
    }
    Ubicacion ubicacion;
    ubicacion.numeroBloque = _obtenerNumeroBloqueLibre(descriptorNodo->bitmap);
    strcpy(ubicacion.nodo, descriptorNodo->nombreNodo);

    if(ubicacion.numeroBloque == -1){
        return -1;
    }

    char data[MB];
    memset(data, 0, sizeof(char) * MB);

    DescriptorNodo * copia0 = (DescriptorNodo *) dictionary_get(nodos, bloque->copia0.nodo);
    DescriptorNodo * copia1 = (DescriptorNodo *) dictionary_get(nodos, bloque->copia1.nodo);

    if(copia0->socket > -1){
        if(_obtenerData(copia0->socket, bloque->copia0.numeroBloque, data) != 0){
            return -1;
        }
    }else if(copia1->socket > -1){
        if(_obtenerData(copia1->socket, bloque->copia1.numeroBloque, data) != 0){
            return -1;
        }
    }else{
        return -1;
    }

    if(_enviarBloque(data, ubicacion) != 0){
        return -1;
    }

    return ubicacion.numeroBloque;
}

int _obtenerData(int socket, int bloque,  char * contenidoBloque){
    int mensaje = GETBLOQUE;

    if (send(socket, &mensaje, sizeof(mensaje), 0) == -1 ||
        send(socket, &bloque, sizeof(bloque), 0) == -1){
        return -1;
    }

    if(recv(socket, contenidoBloque, sizeof(char) * MB, MSG_WAITALL) <= 0){
        return -1;
    }

    return 0;
}

int _controlarEspacioDisponible(int cantidadBloques){
    bloquesLibres = 0;
    dictionary_iterator(nodos,_contarBloquesLibres);

    return 2 * cantidadBloques <= bloquesLibres;
}

void _contarBloquesLibres(char * key, void * nodo){
    DescriptorNodo * descriptorNodo = (DescriptorNodo *) nodo;

    if(descriptorNodo->socket == -1){ // nodo desconectado no lo incluyo en los bloques disponibles
        return;
    }

    bloquesLibres += descriptorNodo->bloquesLibres;
}

int _enviarBloque(char * bloque, Ubicacion ubicacion){
    int mensaje = SETBLOQUE;
    DescriptorNodo * nodo = (DescriptorNodo *) dictionary_get(nodos, ubicacion.nodo);

    printf("Enviando un bloque a %s\n", nodo->nombreNodo);

    if (send(nodo->socket, &mensaje, sizeof(mensaje), 0) == -1 ||
        send(nodo->socket, &ubicacion.numeroBloque, sizeof(ubicacion.numeroBloque), 0) == -1 ||
        send(nodo->socket, bloque, sizeof(char) * MB, 0) ==-1){
        log_error(logger, "No puedo enviar el bloque al nodo %s\n", nodo->nombreNodo);
        return -1;
    }

    return 0;
}

Bloque * _crearBloque(int numeroBloque, long bytes){
    Bloque * bloque = (Bloque *) malloc(sizeof(*bloque));
    bloque->descriptor.numeroBloque = numeroBloque;
    bloque->descriptor.bytes = bytes;

    _calcularUbicacionBloque(bloque);

    bloque->otrasCopias = NULL;
    return bloque;
}

void _calcularUbicacionBloque(Bloque * bloque){
    int cantidadNodos = list_size(nombreNodos);
    int copiasUbicadas = 0;

    if(nodoConectado != NULL){
        strcpy(bloque->copia0.nodo, nodoConectado->nombreNodo);
        bloque->copia0.numeroBloque = -2;
        strcpy(bloque->copia1.nodo, nodoConectado->nombreNodo);
        bloque->copia1.numeroBloque = -2;
        nodoConectado->bloquesLibres -= 2;
        return;
    }

    while(copiasUbicadas < 2){
        DescriptorNodo * descriptorNodo = dictionary_get(nodos, list_get(nombreNodos, punteroNodo));

        if(descriptorNodo->bloquesLibres > 0 && descriptorNodo->socket > -1){
            if(copiasUbicadas == 0){
                strcpy(bloque->copia0.nodo, descriptorNodo->nombreNodo);
                bloque->copia0.numeroBloque = _obtenerNumeroBloqueLibre(descriptorNodo->bitmap);
            }else{
                strcpy(bloque->copia1.nodo, descriptorNodo->nombreNodo);
                bloque->copia1.numeroBloque = _obtenerNumeroBloqueLibre(descriptorNodo->bitmap);
            }

            descriptorNodo->bloquesLibres--;
            copiasUbicadas++;
        }

        punteroNodo = (punteroNodo + 1) % cantidadNodos;
    }
}

int _obtenerNumeroBloqueLibre(t_bitarray * bitmap) {
    int i = 0,
            numeroBloque = -1;

    while (numeroBloque == -1) {
        if (bitarray_test_bit(bitmap, i) == 0) {
            bitarray_set_bit(bitmap, i);
            numeroBloque = i;
        }
        i++;
    }

    return numeroBloque;
}

int _manejarEnvioBloque(char * bloque, Bloque * descriptorBloque){
    int resultado = 0;
    if(_enviarBloque(bloque, descriptorBloque->copia0) != 0){
        return -1;
    }

    DescriptorNodo * nodo = (DescriptorNodo *) dictionary_get(nodos, descriptorBloque->copia0.nodo);

    zrecv(nodo->socket, &resultado, sizeof(resultado), 0);

    if(!resultado){
        return -1;
    }

    if(_enviarBloque(bloque, descriptorBloque->copia1) != 0){
        return -1;
    }

    nodo = (DescriptorNodo *) dictionary_get(nodos, descriptorBloque->copia1.nodo);

    resultado = 0;
    zrecv(nodo->socket, &resultado, sizeof(resultado), 0);

    if(!resultado){
        return -1;
    }
    return 0;
}
