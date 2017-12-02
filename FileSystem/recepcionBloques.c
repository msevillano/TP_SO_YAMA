#include "recepcionBloques.h"
#include "envioBloques.h"
#include <sys/socket.h>
#include "conexionesFileSystem.h"
#include <string.h>
#include <stdlib.h>

int _guardarBloque(InfoRecepcion ** info);
int * _manejarGuardarBloque(InfoRecepcion * infoRecepcion);
int _seleccionarNodoLectura(Bloque * bloque, DescriptorNodo ** nodo);

int obtenerArchivo(char * rutaArchivoFS, char * rutaArchivoFinal){
    Archivo * descriptorArchivo = (Archivo *) dictionary_get(archivos, rutaArchivoFS);

    if(descriptorArchivo == NULL){
        log_error(logger, "El archivo %s no existe en YAMAFS\n", rutaArchivoFS);
        return -1;
    }

    FILE * archivoFinal = fopen(rutaArchivoFinal, "w");

    if(archivoFinal == NULL){
        log_error(logger, "No se puede crear el archivo %s\n", rutaArchivoFinal);
        return -1;
    }

    int i;
    int bloques = list_size(descriptorArchivo->bloques);

    InfoRecepcion * infoRecepcion = (InfoRecepcion *) malloc(sizeof(*infoRecepcion) *bloques);
    pthread_t * tid = (pthread_t *) malloc(sizeof(*tid) *bloques);;

    for(i = 0; i < bloques; ++i){
        Bloque * bloque = (Bloque *) list_get(descriptorArchivo->bloques, i);

        infoRecepcion[i].nodo = NULL;

        int numBloque = _seleccionarNodoLectura(bloque, &infoRecepcion[i].nodo);

        if(numBloque == -1){
            log_error(logger, "El bloque %d del archivo %s no tiene copias disponibles\n", i, rutaArchivoFS);
            return -1;
        }

        memset(infoRecepcion[i].datos, 0, sizeof(char) * MB);
        infoRecepcion[i].bloque = numBloque;
        infoRecepcion[i].bytes = bloque->descriptor.bytes;

        pthread_mutex_lock(&infoRecepcion[i].nodo->semaforo);
        pthread_create(&tid[i], NULL, _manejarGuardarBloque, &infoRecepcion[i]);
    }

    int index;
    int errores = 0;

    for(index = 0; index < bloques; ++index){
        int * retval;
        pthread_join(tid[index], &retval);


        if(*retval != 0){
            errores = 1;
        }else if(fwrite(infoRecepcion[index].datos, sizeof(char) * infoRecepcion[index].bytes, 1, archivoFinal) != 1){
            errores = 1;
        }

        free(retval);
    }

    free(infoRecepcion);
    free(tid);

    fclose(archivoFinal);

    if(errores == 1){
        remove(archivoFinal);
    }
    return 0;
}

int * _manejarGuardarBloque(InfoRecepcion * infoRecepcion){
    int * status = (int *) malloc(sizeof(int));
    *status = _guardarBloque(&infoRecepcion);

    return status;
}

int _guardarBloque(InfoRecepcion ** info){
    InfoRecepcion * infoRecepcion = *info;
    DescriptorNodo * nodo = infoRecepcion->nodo;
    int bloque = infoRecepcion->bloque;
    int mensaje = GETBLOQUE;

    if (send(nodo->socket, &mensaje, sizeof(mensaje), 0) == -1 ||
        send(nodo->socket, &bloque, sizeof(bloque), 0) == -1){
        log_error(logger, "No puedo solicitar el bloque al nodo %s\n", nodo->nombreNodo);
        return -1;
    }

    printf("Recibiendo un bloque de %s\n", nodo->nombreNodo);

    if(recv(nodo->socket, infoRecepcion->datos, sizeof(char) * MB, MSG_WAITALL) <= 0){
        log_error(logger, "Error recibiendo el bloque del nodo %s\n", nodo->nombreNodo);
        return -1;
    }

    pthread_mutex_unlock(&nodo->semaforo);
    return 0;
}

int _seleccionarNodoLectura(Bloque * bloque, DescriptorNodo ** nodo){
    int cantidadNodos = list_size(nombreNodos);
    int copiasDesconectadas = 0;
    int cantidadCopias = 2;

    if(bloque->otrasCopias != NULL){
        cantidadCopias += list_size(bloque->otrasCopias);
    }

    while(copiasDesconectadas < cantidadCopias){
        DescriptorNodo * descriptorNodo = dictionary_get(nodos, list_get(nombreNodos, punteroNodo));

        punteroNodo = (punteroNodo + 1) % cantidadNodos;

        if(descriptorNodo == NULL){
            continue;
        }

        if(strcmp(bloque->copia0.nodo, descriptorNodo->nombreNodo) == 0){
            if(descriptorNodo->socket == -1 || bloque->copia0.numeroBloque == -1){
                copiasDesconectadas++;
            }else{
                *nodo = descriptorNodo;
                return bloque->copia0.numeroBloque;
            }
        }else if(strcmp(bloque->copia1.nodo, descriptorNodo->nombreNodo) == 0){
            if(descriptorNodo->socket == -1 || bloque->copia1.numeroBloque == -1){
                copiasDesconectadas++;
            }else{
                *nodo = descriptorNodo;
                return bloque->copia1.numeroBloque;
            }
        }else if(bloque->otrasCopias != NULL){
            bool buscarCopia(void * ubicacion){
                Ubicacion * copia = (Ubicacion *)ubicacion;

                return strcmp(copia->nodo, descriptorNodo->nombreNodo) == 0;
            }

            Ubicacion * ubicacion = (Ubicacion *)list_find(bloque->otrasCopias, buscarCopia);

            if(ubicacion != NULL){
                if(descriptorNodo->socket == -1){
                    copiasDesconectadas++;
                }else{
                    *nodo = descriptorNodo;
                    return ubicacion->numeroBloque;
                }
            }
        }
    }

    return -1;
}
