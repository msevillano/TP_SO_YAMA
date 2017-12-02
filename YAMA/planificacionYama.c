#include "planificacionYama.h"
#include "servidor.h"
#include "serializacion.h"
#include "yama.h"
#include <string.h>
#include <sys/time.h>
#include <sockets.h>
#include <protocoloComunicacion.h>

int _buscarMax(t_list * nodos);
int _pWl(InfoNodo * nodo, int max);
void _setAvailability(t_list * nodos);
void _sumarTrabajos(t_list * nodos);
void _enviarAMaster(int socket_master, InfoNodo * nodo, InfoNodo * nodoCopia, TamanoBloque * bloque, TipoOperacion operacion, char* bloqueArchivo);
void _sumarDisponiblidadBase(t_list * listaNodos);
void _sortNodos(t_list * nodos);


void ordenarNodos(t_list * nodos) {
	_sumarTrabajos(nodos);
	_setAvailability(nodos);
	_sortNodos(nodos);
}


InfoNodo * buscarCopia(char numeroBloqueStr[5], t_list * listaNodos){
	int encontrado = 0, i = 0;
	while (!encontrado && i < list_size(listaNodos)) {
		InfoNodo * nodo = (InfoNodo *) list_get(listaNodos, i);
		TamanoBloque * bloque = (TamanoBloque *) dictionary_get(nodo->bloques,
				numeroBloqueStr);
		if (bloque != NULL) {
			encontrado = 1;
			return nodo;
		}
		i++;
	}
	return NULL;
}

int trabajoActual(char* nombreNodo){
	int trabajo = 0;

	void iterator(void* ent) {
		EntradaTablaEstado* en = (EntradaTablaEstado*) ent;
		if (!strcmp(en->nombreNodo, nombreNodo) && en->estado == ENPROCESO){
				if (en->etapa == REDUCGLOBAL) {

				bool criterioFilter(void * entrada) {
					EntradaTablaEstado * entradaTablaEstado =
							(EntradaTablaEstado *) entrada;

					return entradaTablaEstado->jobId == en->jobId
							&& entradaTablaEstado->etapa == REDUCCIONLOCAL;
				}

				t_list * reducciones = list_filter(tablaEstado, criterioFilter);

				trabajo = list_size(reducciones) / 2;
				trabajo += list_size(reducciones) % 2;

			} else
				trabajo++;
		}
	}
	list_iterate(tablaEstado, iterator);
	return trabajo;
}

void replanificar(int socket){
	usleep(retardoPlanificacion * 1000);
	InfoNodo* nodoCopia = NULL;
	transfError transf;
	int jobId;
	yrecv(socket, &transf, sizeof(transf), 0);
	TamanoBloque * bloque = (TamanoBloque *) malloc(sizeof(*bloque));
	int replanificadas = 0;

	void buscarNodoCopia(void * entrada){
		EntradaTablaEstado * en = (EntradaTablaEstado *) entrada;

		if(!strcmp(en->nombreNodo, transf.nombreNodo) &&
			en->numeroBloque == transf.numBloque &&
			!strcmp(en->archivoTemporal, transf.nombreTemp)){

			jobId = en->jobId;
			en->estado = ERRORYAMA;
			nodoCopia = en->nodoCopia;
			if(nodoCopia!=NULL)
				bloque = dictionary_remove(en->nodoCopia->bloques, en->bloqueArchivo);

			logEntrada(en->masterId, en->jobId, en->disponibilidad,
					trabajoActual(transf.nombreNodo), "REPLANIFICANDO",
					en->nombreNodo, en->nodoCopia->nombre, transf.numBloque,
					"TRANSFORMACION", transf.nombreTemp);
		}
	}

	list_iterate(tablaEstado, buscarNodoCopia);

	SocketJob* job;
	bool closure(void* sockJob) {
		SocketJob* sj = (SocketJob*) sockJob;
		return sj->socket == socket;
	}
	job = (SocketJob*) list_find(socketJobs, closure);

	void matarPorReplanificacion() {
		int muerte = FALLOTRANSFORMACION;

		void ajustarCarga(void* entrada) {
			EntradaTablaEstado* en = (EntradaTablaEstado*) entrada;
			if (jobId == en->jobId && en->estado != ERRORYAMA) {
				en->estado = FINALIZADO;
			}
		}
		list_iterate(tablaEstado, ajustarCarga);

		if (job != NULL && !replanificadas && job != NULL) {
			ysend(socket, &muerte, sizeof(int), 0);
			log_error(logger, "No se puede replanificar la transformacion. Master killed");
			desasociarSocketJob(socket);
			close(socket);
			FD_CLR(socket, &master);
			replanificadas = 1;
			cabecera = 0;
		}
	}

	if (nodoCopia != NULL) {

		int tipoOperacion = REPLANIFICACION;

		ysend(socket, &tipoOperacion, sizeof(tipoOperacion), 0);

		_enviarAMaster(socket, nodoCopia, NULL, bloque, TRANSFORMACION, NULL);

		void replanificarFinalizadas(void* entrada) {
			EntradaTablaEstado* en = (EntradaTablaEstado*) entrada;

			if (jobId == en->jobId && !strcmp(transf.nombreNodo, en->nombreNodo)
					&& en->estado == FINALIZADO && !replanificadas && job!=NULL) {
				usleep(retardoPlanificacion * 1000);
				en->estado = ERRORYAMA;
				if (en->nodoCopia != NULL) {
					TamanoBloque * block = dictionary_remove(en->nodoCopia->bloques, en->bloqueArchivo);
					if (block !=NULL){
						logEntrada(en->masterId, en->jobId, en->disponibilidad,
							0, "REPLANIFICANDO",
							en->nombreNodo, en->nodoCopia->nombre, en->numeroBloque,
							"TRANSFORMACION", en->archivoTemporal);

					ysend(socket, &tipoOperacion, sizeof(tipoOperacion), 0);
					_enviarAMaster(socket, en->nodoCopia, NULL, block,
							TRANSFORMACION, NULL);
					}

				} else {
					matarPorReplanificacion();
					return;
				}
			}
		}

		list_iterate(tablaEstado, replanificarFinalizadas);

	} else
		matarPorReplanificacion();
	free(bloque);
}

void planificarBloquesYEnviarAMaster(int socket_master, int bloques, t_list * listaNodos){
	usleep(retardoPlanificacion * 1000);
	int i;
	int clock_ = 0;
	int cantidadNodos = list_size(listaNodos);

	/*log_info(logger, "Disponibilidades inicial:");
	void printDispI(void*nodo){
		InfoNodo*n=(InfoNodo*) nodo;
		log_info(logger, "%s: %d", n->nombre, n->disponibilidad);
	}
	list_iterate(listaNodos, printDispI);
	cabecera = 0;//si borras el log de disponibilidades, borra esto tmb (es la cabecera de la tabla de estados)
	 */
	for(i = 0; i < bloques; ++i){
		int inicial = clock_;
		int planificado = 0;
		int fallos = 0;

		while(!planificado){

			InfoNodo * nodo = (InfoNodo *) list_get(listaNodos, clock_), *nodoCopia;
			char numeroBloqueStr[5];
			intToString(i, numeroBloqueStr);

			TamanoBloque * bloque = (TamanoBloque *) dictionary_get(nodo->bloques,numeroBloqueStr);

			//printf("Clock: %d, Inicial: %d, fallo: %d, %s Disp: %d\n", clock_, inicial, fallos, nodo->nombre, nodo->disponibilidad);

			if (bloque != NULL) {
				if (nodo->disponibilidad == 0) {
					nodo->disponibilidad += disponibilidad_base;
					fallos = 1;
				} else {
					if (clock_ == inicial && fallos) {
						_sumarDisponiblidadBase(listaNodos);
					}
					dictionary_remove(nodo->bloques,numeroBloqueStr);
					nodoCopia = buscarCopia(numeroBloqueStr, listaNodos);
					_enviarAMaster(socket_master, nodo, nodoCopia, bloque,
							TRANSFORMACION, numeroBloqueStr);
					nodo->disponibilidad--;
					planificado = 1;
					usleep(retardoPlanificacion * 1000);
				}
			}
			clock_ = (clock_ + 1) % cantidadNodos;
		}
	}
	/*log_info(logger, "Disponibilidades final:");
	void printDispF(void*nodo){
		InfoNodo*n=(InfoNodo*) nodo;
		log_info(logger, "%s: %d", n->nombre, n->disponibilidad);
	}
	list_iterate(listaNodos, printDispF);

	cabecera = 0; //si borras el log de disponibilidades, borra esto tmb (es la cabecera de la tabla de estados)
	*/
}

void logEntrada(int masterId, int jobId, int disp, int trabajo, char*estado,
		char* nombreNodo, char*nombreCopia, int numBloque, char* etapa,
		char* archTemp) {
	if (!cabecera) {
		log_info(logger,
				"    \tMaster\tJobId\tDisp\tCarga\tEstado\t\tNodo\tCopia\tBloque\tEtapa\t\tTemporal");
		cabecera = 1;
	}
	if (!strcmp("TRANSFORMACION", etapa)) {
		if (disp == 0) {
			log_info(logger, "    \t%d\t%d\t\t%d\t%s\t%s\t%s\t%d\t%s\t%s",
					masterId, jobId, trabajo, estado, nombreNodo, nombreCopia,
					numBloque, etapa, archTemp);
		} else {
			log_info(logger, "    \t%d\t%d\t%d\t%d\t%s\t%s\t%s\t%d\t%s\t%s",
					masterId, jobId, disp, trabajo, estado, nombreNodo,
					nombreCopia, numBloque, etapa, archTemp);
		}

	} else {
		log_info(logger, "    \t%d\t%d\t\t%d\t%s\t%s\t%s\t\t%s\t%s", masterId,
				jobId, trabajo, estado, nombreNodo, nombreCopia,
				etapa, archTemp);
	}
}

void _enviarAMaster(int socket_master, InfoNodo * nodo, InfoNodo * nodoCopia, TamanoBloque * bloque, TipoOperacion operacion, char*bloqueArchivo){
	ysend(socket_master, nodo->nombre, sizeof(char)*100, 0);
	ysend(socket_master, nodo->ip, sizeof(char)*20, 0);
	ysend(socket_master, &nodo->puerto, sizeof(nodo->puerto), 0);
	ysend(socket_master, &bloque->bloque, sizeof(bloque->bloque), 0);
	ysend(socket_master, &bloque->bytes, sizeof(bloque->bytes), 0);

	char tempFile[255];
	memset(tempFile, 0, sizeof(char) * 255);

	generarArchivoTemporal(nodo->nombre, tempFile);

	ysend(socket_master, tempFile, 255 * sizeof(char), 0);

	EntradaTablaEstado * entradaTablaEstado = (EntradaTablaEstado *)malloc(sizeof(*entradaTablaEstado));

	SocketJob* job;
	bool closure(void* sockJob) {
		SocketJob* sj = (SocketJob*) sockJob;
		return sj->socket == socket_master;
	}
	job = (SocketJob*) list_find(socketJobs, closure);

	entradaTablaEstado->masterId = job->jobId;
	entradaTablaEstado->jobId = job->jobId;
	entradaTablaEstado->estado = ENPROCESO;
	strcpy(entradaTablaEstado->nombreNodo, nodo->nombre);
	entradaTablaEstado->numeroBloque = bloque->bloque;
	entradaTablaEstado->tamBloque = bloque->bytes;
	entradaTablaEstado->etapa = operacion;
	strcpy(entradaTablaEstado->archivoTemporal, tempFile);
	strcpy(entradaTablaEstado->ip, nodo->ip);
	entradaTablaEstado->puerto = nodo->puerto;
	entradaTablaEstado->nodoCopia = nodoCopia;
	entradaTablaEstado->disponibilidad = nodoCopia == NULL? 0 : nodo->disponibilidad;
	entradaTablaEstado->historicas = getTareasHistoricas(nodo->nombre);
	if(bloqueArchivo!=NULL) strcpy(entradaTablaEstado->bloqueArchivo, bloqueArchivo);


	list_add(tablaEstado, entradaTablaEstado);

	logEntrada(entradaTablaEstado->masterId, entradaTablaEstado->jobId,
			entradaTablaEstado->disponibilidad, trabajoActual(nodo->nombre), "EN PROCESO",
			entradaTablaEstado->nombreNodo,
			entradaTablaEstado->nodoCopia->nombre,
			entradaTablaEstado->numeroBloque, "TRANSFORMACION",
			entradaTablaEstado->archivoTemporal);

	entradaTablaEstado->disponibilidad = 0;

}

void generarArchivoTemporal(char * nombre, char * file){
	struct timeval tv;
	gettimeofday(&tv,NULL);

	sprintf(file, "temp/%s-%ld.%06ld-%d", nombre, tv.tv_sec, tv.tv_usec, list_size(tablaEstado));
}

void _setAvailability(t_list * nodos){
	if(algoritmoBalanceo == WEIGHTEDCLOCK){
		int max = _buscarMax(nodos);
		void availability(void * nodo){
			InfoNodo * infoNodo = (InfoNodo *) nodo;
			infoNodo->disponibilidad += _pWl(infoNodo, max);
		}
		list_iterate(nodos, availability);
	}
}

void _sumarTrabajos(t_list * nodos){
	void sumarTrabajo(void * entrada){
		EntradaTablaEstado * en = (EntradaTablaEstado *) entrada;
		if(en->estado == ENPROCESO){
			bool buscarEnListaNodos(void * nodo){
				InfoNodo * in = (InfoNodo *) nodo;
				return strcmp(en->nombreNodo, in->nombre) == 0;
			}
			InfoNodo * nodo = list_find(nodos,buscarEnListaNodos);
			if(nodo != NULL){
				nodo->trabajosActuales ++;
			}
		}
	}
	list_iterate(tablaEstado,sumarTrabajo);
}

int _buscarMax(t_list * nodos){
	int max = 0;
	int cantNodos = list_size(nodos);
	int i;
	for(i = 0; i < cantNodos; ++i){
		InfoNodo * nodo = (InfoNodo *) list_get(nodos, i);
		if(nodo->trabajosActuales > max){
			max = nodo->trabajosActuales;
		}
	}
	return max;
}

int _pWl(InfoNodo * nodo, int max){
	return max - nodo->trabajosActuales;
}

void _sumarDisponiblidadBase(t_list * listaNodos){
	void sumarDisponibilidad(void * nodo){
		InfoNodo * in = (InfoNodo *) nodo;
		in->disponibilidad += disponibilidad_base;
	}
	list_iterate(listaNodos, sumarDisponibilidad);
}

void _sortNodos(t_list * nodos){
	bool _ordenarPorDisponibilidad(void * n1, void * n2){
		InfoNodo * nodo1 = (InfoNodo *) n1,
				* nodo2 = (InfoNodo *) n2;

		if(nodo1->disponibilidad == nodo2->disponibilidad){
			return getTareasHistoricas(nodo1->nombre) <  getTareasHistoricas(nodo2->nombre);
		}

		return nodo1->disponibilidad > nodo2->disponibilidad;
	}

	list_sort(nodos, _ordenarPorDisponibilidad);
}

int getTareasHistoricas(char * nombreNodo){
	int tareas = 0;
	void sumarTrabajoHistoricos(void * entrada){
		EntradaTablaEstado * en = (EntradaTablaEstado *) entrada;

		if(en->estado != ENPROCESO && strcmp(en->nombreNodo, nombreNodo) == 0){
			tareas++;
		}
	}

	list_iterate(tablaEstado, sumarTrabajoHistoricos);

	return tareas;
}
