#include "respuestaTransformacion.h"
#include "planificacionYama.h"
#include <sockets.h>
#include "yama.h"
#include <string.h>
#include "serializacion.h"
#include "RGYAMA.h"
#include <protocoloComunicacion.h>

void _enviarOperacionAlmacenamiento(int socket, EntradaTablaEstado * reduccion);

void almacenamientoOK(int socket){
    int jobId;

    zrecv(socket, &jobId, sizeof(jobId), 0);

    bool criterioBusqueda(void * entrada){
        EntradaTablaEstado * entradaTablaEstado = (EntradaTablaEstado * )entrada;

        return entradaTablaEstado->jobId == jobId && entradaTablaEstado->etapa == ALMACENAMIENTOOP;
    }

    EntradaTablaEstado * entradaFinalizada = list_find(tablaEstado, criterioBusqueda);

    if(entradaFinalizada != NULL) {
        entradaFinalizada->estado = FINALIZADO;

        logEntrada(entradaFinalizada->masterId, entradaFinalizada->jobId,
        				entradaFinalizada->disponibilidad,
        				trabajoActual(entradaFinalizada->nombreNodo), "FINALIZADO",
        				entradaFinalizada->nombreNodo, "-",
        				0, "ALMACENAMIENTO",
        				entradaFinalizada->archivoTemporal);

        log_info(logger, "El job %d ha sido finalizado", jobId);
        cabecera = 0;
    }
}

void transformacionOK(int socket){
    int jobId;
    char tempFile[255];
    zrecv(socket, &jobId, sizeof(jobId), 0);
    zrecv(socket, tempFile, sizeof(char) * 255, 0);

    bool criterioBusqueda(void * entrada){
        EntradaTablaEstado * entradaTablaEstado = (EntradaTablaEstado * )entrada;

        return strcmp(tempFile, entradaTablaEstado->archivoTemporal) == 0;
    }

    EntradaTablaEstado * entradaFinalizada = list_find(tablaEstado, criterioBusqueda);

    if(entradaFinalizada != NULL){
        entradaFinalizada->estado = FINALIZADO;

		logEntrada(entradaFinalizada->masterId, entradaFinalizada->jobId,
				entradaFinalizada->disponibilidad,
				trabajoActual(entradaFinalizada->nombreNodo), "FINALIZADO",
				entradaFinalizada->nombreNodo, entradaFinalizada->nodoCopia->nombre,
				entradaFinalizada->numeroBloque, "TRANSFORMACION",
				entradaFinalizada->archivoTemporal);

		bool criterioFilter(void * entrada) {
			EntradaTablaEstado * entradaTablaEstado = (EntradaTablaEstado *)entrada;

            return entradaTablaEstado->jobId == jobId &&
                   entradaTablaEstado->etapa == TRANSFORMACION &&
                   strcmp(entradaTablaEstado->nombreNodo, entradaFinalizada->nombreNodo) == 0;
        }

        t_list * transformaciones = list_filter(tablaEstado, criterioFilter);

        bool estanFinalizadas(void * entrada){
            EntradaTablaEstado * entradaTablaEstado = (EntradaTablaEstado *)entrada;

            return entradaTablaEstado->estado == FINALIZADO;
        }

        if(list_all_satisfy(transformaciones, estanFinalizadas)){
            enviarSolicitudReduccion(socket, transformaciones);
        }
    }else{
        log_error(logger, "NO ENCONTRO LA Transformacion\njob: %d\ntempFile: %s\n", jobId, tempFile);
        cabecera = 0;
    }
}

void reduccionLocalOK(int socket){
    int jobId;
    char tempFile[255];
    zrecv(socket, &jobId, sizeof(jobId), 0);
    zrecv(socket, tempFile, sizeof(char) * 255, 0);

    bool criterioBusqueda(void * entrada){
        EntradaTablaEstado * entradaTablaEstado = (EntradaTablaEstado * )entrada;

        return strcmp(tempFile, entradaTablaEstado->archivoTemporal) == 0;
    }

    EntradaTablaEstado * entradaFinalizada = list_find(tablaEstado, criterioBusqueda);


    if(entradaFinalizada != NULL){
    	entradaFinalizada->estado = FINALIZADO;

        void cancelarFinalizadasAnteriores(void* entrada){
        	EntradaTablaEstado * en = (EntradaTablaEstado *)entrada;
			if (en->jobId == jobId && en->etapa == REDUCCIONLOCAL
					&& !strcmp(en->nombreNodo, entradaFinalizada->nombreNodo)
					&& en->estado == FINALIZADO &&
					strcmp(en->archivoTemporal, entradaFinalizada->archivoTemporal)) {
				en->estado = ERRORYAMA;
			}
        }
        list_iterate(tablaEstado, cancelarFinalizadasAnteriores);

        logEntrada(entradaFinalizada->masterId, entradaFinalizada->jobId,
        				entradaFinalizada->disponibilidad,
        				trabajoActual(entradaFinalizada->nombreNodo), "FINALIZADO",
        				entradaFinalizada->nombreNodo, "-",
        				entradaFinalizada->numeroBloque, "REDUCCION LOCAL",
        				entradaFinalizada->archivoTemporal);

        bool criterioFilter(void * entrada){
            EntradaTablaEstado * entradaTablaEstado = (EntradaTablaEstado *)entrada;

			return entradaTablaEstado->jobId == jobId
					&& !(entradaTablaEstado->etapa == REDUCCIONLOCAL
							&& entradaTablaEstado->estado == ERRORYAMA);
		}

        t_list * reducciones = list_filter(tablaEstado, criterioFilter);

        bool estanFinalizadas(void * entrada){
            EntradaTablaEstado * entradaTablaEstado = (EntradaTablaEstado *)entrada;

            return entradaTablaEstado->estado == FINALIZADO ||
                   (entradaTablaEstado->etapa == TRANSFORMACION && entradaTablaEstado->estado == ERRORYAMA);
        }

		if (list_all_satisfy(reducciones, estanFinalizadas))
			reduccionGlobal(socket, jobId);

	} else {
		log_error(logger,
				"NO ENCONTRO LA Reduccion Local\njob: %d\ntempFile: %s\n",
				jobId, tempFile);
		cabecera = 0;
	}
}

void reduccionGlobalOk(int socket){
    int jobId;

    zrecv(socket, &jobId, sizeof(jobId), 0);

    bool criterioBusqueda(void * entrada){
        EntradaTablaEstado * entradaTablaEstado = (EntradaTablaEstado * )entrada;

        return entradaTablaEstado->jobId == jobId && entradaTablaEstado->etapa == REDUCGLOBAL;
    }

    EntradaTablaEstado * reduccionGlobal = list_find(tablaEstado, criterioBusqueda);

    if(reduccionGlobal != NULL){
        reduccionGlobal->estado = FINALIZADO;

		logEntrada(reduccionGlobal->masterId, reduccionGlobal->jobId,
				reduccionGlobal->disponibilidad,
				trabajoActual(reduccionGlobal->nombreNodo), "FINALIZADO",
				reduccionGlobal->nombreNodo, "-",
				reduccionGlobal->numeroBloque, "REDUC. GLOBAL",
				reduccionGlobal->archivoTemporal);

        _enviarOperacionAlmacenamiento(socket, reduccionGlobal);
    }else{
        log_error(logger, "NO ENCONTRO LA Reduccion Global\njob: %d\n", jobId);
        cabecera = 0;
    }
}

void _enviarOperacionAlmacenamiento(int socket, EntradaTablaEstado * reduccion){
    EntradaTablaEstado * en = (EntradaTablaEstado *) malloc(sizeof(*en));

    en->puerto = reduccion->puerto;
    en->estado = ENPROCESO;
    en->etapa = ALMACENAMIENTOOP;
    en->numeroBloque = 0;
    en->jobId = reduccion->jobId;
    en->masterId = reduccion->masterId;
    strcpy(en->nombreNodo, reduccion->nombreNodo);
    strcpy(en->ip, reduccion->ip);
    en->disponibilidad = reduccion->disponibilidad;

    memset(en->archivoTemporal, 0, sizeof(char) * 255);

    list_add(tablaEstado, en);

	logEntrada(en->masterId, en->jobId, en->disponibilidad,(int) trabajoActual(reduccion->nombreNodo),
			"EN PROCESO", en->nombreNodo, "-", 0, "ALMACENAMIENTO",
			en->archivoTemporal);

    int almacenamiento = SOLICITUDALMACENAMIENTO;
    zsend(socket, &almacenamiento, sizeof(almacenamiento), 0);
    zsend(socket, en->nombreNodo, sizeof(char) * 100,  0);
    zsend(socket, en->ip, sizeof(char) * 20, 0);
    zsend(socket, &en->puerto, sizeof(en->puerto), 0);
    zsend(socket, reduccion->archivoTemporal, sizeof(char) * 255, 0);
}
