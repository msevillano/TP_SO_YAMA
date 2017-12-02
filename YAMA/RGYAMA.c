#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sockets.h>
#include "yama.h"
#include <protocoloComunicacion.h>
#include "planificacionYama.h"

int calcularIndiceConValorMinimo(int * array, int cantidad);

void reduccionGlobal(int socket, int jobId){

	bool criterioFilter(void * entrada){
		EntradaTablaEstado * entradaTablaEstado = (EntradaTablaEstado *)entrada;

		return entradaTablaEstado->jobId == jobId &&
			   entradaTablaEstado->etapa == REDUCCIONLOCAL &&
			   entradaTablaEstado->estado == FINALIZADO;
	}

	t_list * reducciones = list_filter(tablaEstado, criterioFilter);
	int cantidad = list_size(reducciones);

	int trabajosActuales[cantidad];
	memset(trabajosActuales, 0, cantidad * sizeof(int));

	void iterateFunction(void * entrada){
		EntradaTablaEstado * entradaTablaEstado = (EntradaTablaEstado *)entrada;

		if(entradaTablaEstado->estado == ENPROCESO){
			int i;
			for(i = 0; i < cantidad; ++i){
				EntradaTablaEstado * nodo = (EntradaTablaEstado *) list_get(reducciones, i);

				if(strcmp(nodo->nombreNodo, entradaTablaEstado->nombreNodo) == 0){
					if(entradaTablaEstado->etapa == REDUCGLOBAL){
						trabajosActuales[i] = list_size(reducciones)/2;
						trabajosActuales[i] += list_size(reducciones) % 2;
					}
					else{
						trabajosActuales[i]++;
					}
				}
			}
		}
	}

	list_iterate (tablaEstado, iterateFunction);

	int minIndex = calcularIndiceConValorMinimo(trabajosActuales, cantidad);

	int tipoMensaje = SOLICITUDREDUCCIONGLOBAL;

	ysend(socket, &tipoMensaje, sizeof(tipoMensaje), 0);

	ysend(socket, &cantidad, sizeof(cantidad), 0);

	int i;

	EntradaTablaEstado * en = (EntradaTablaEstado *) malloc(sizeof(*en));

	for(i = 0; i < cantidad; ++i){
		EntradaTablaEstado * entradaNodo = list_get(reducciones, i);

		int esEncargado = i == minIndex ? 1 : 0;

		ysend(socket, &esEncargado, sizeof(esEncargado), 0);
		ysend(socket, entradaNodo->nombreNodo, sizeof(char) * 100, 0);
		ysend(socket, entradaNodo->ip, sizeof(char) * 20, 0);
		ysend(socket, &entradaNodo->puerto, sizeof(entradaNodo->puerto), 0);
		ysend(socket, entradaNodo->archivoTemporal, sizeof(char) * 255, 0);


		if(esEncargado){
			en->puerto = entradaNodo->puerto;
			en->estado = ENPROCESO;
			en->etapa = REDUCGLOBAL;
			en->numeroBloque = 0;
			en->jobId = entradaNodo->jobId;
			en->masterId = entradaNodo->masterId;
			strcpy(en->nombreNodo, entradaNodo->nombreNodo);
			strcpy(en->ip, entradaNodo->ip);
			en->disponibilidad = entradaNodo->disponibilidad;
			generarArchivoTemporal(en->nombreNodo, en->archivoTemporal);
		}
	}

	ysend(socket, en->archivoTemporal, sizeof(char) * 255, 0);

	list_add(tablaEstado, en);

	logEntrada(en->masterId, en->jobId, en->disponibilidad, (int) trabajoActual(en->nombreNodo), "EN PROCESO", en->nombreNodo, "-", "-",
			"REDUC. GLOBAL", en->archivoTemporal);
}

int calcularIndiceConValorMinimo(int * array, int cantidad){
	int i;
	int min = *array;
	int index = 0;

	for(i = 1; i < cantidad; ++i){
		if(array[i] < min){
			index = i;
			min = array[i];
		}
	}

	return index;
}
