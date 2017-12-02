#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "servidor.h"
#include "chat.h"
#include "yama.h"
#include "clienteFS.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "planificacionYama.h"

void signal_handler(int signum){
	if (signum == SIGUSR1) {
		recargarConfiguracion();
		log_info(logger, "Recarga de configurcion OK\n");
		cabecera = 0;
	}
}

void logEstadoNodos(){
	/////// logTablaEstados

	log_info(logger, "\n\nTabla Estados:\n");

	void etapa(int etapa, char* etp){
		switch (etapa) {
		case 1:
			strcpy(etp, "TRANSFORMACION");
			break;
		case 2:
			strcpy(etp, "REDUCCION LOCAL");
			break;
		case 3:
			strcpy(etp, "REDUC. GLOBAL");
			break;
		case 4:
			strcpy(etp, "ALMACENAMIENTO");
			break;
		default:
			strcpy(etp, "NI IDEA CHE");
			break;
		}
	}

	void state(int estado, char* est) {
		switch (estado) {
		case 1:
			strcpy(est, "EN PROCESO");
			break;
		case 2:
			strcpy(est, "ERROR     ");
			break;
		case 3:
			strcpy(est, "FINALIZADO");
			break;
		default:
			strcpy(est, "NI IDEA MAN");
			break;
		}
	}

	void logEn(void* entrada){

		EntradaTablaEstado* en = (EntradaTablaEstado*) malloc(sizeof(*en));
		en = (EntradaTablaEstado*) entrada;
		char nombreCopia[260], archTemp[100], nombreNodo[100];
		if (en->nombreNodo != NULL)
			strcpy(nombreNodo, en->nombreNodo);
		else
			strcpy(nombreNodo, "no name");

		if (en->archivoTemporal != NULL)
			strcpy(archTemp, en->archivoTemporal);
		else
			strcpy(archTemp, "no temp");

		char * estado, *etp;
		estado = (char*) malloc(sizeof(char)*30);
		etp = (char*) malloc(sizeof(char)*30);
		state(en->estado, estado);
		etapa(en->etapa, etp);

		if (!cabecera) {
			log_info(logger,
					"   \tMaster\tJobId\tDisp\tCarga\tEstado\t\tNodo\tCopia\tBloque\tEtapa\t\tTemporal");
			cabecera = 1;
		}
		strcpy(nombreCopia, "-");
		if (!strcmp("TRANSFORMACION", etp)) {
			if (en->disponibilidad == 0) {
				log_info(logger, "    \t%d\t%d\t\t%d\t%s\t%s\t%s\t%d\t%s\t%s",
						en->masterId, en->jobId, 0, estado, nombreNodo, nombreCopia,
						en->numeroBloque, etp, archTemp);
			} else {
				log_info(logger, "   \t%d\t%d\t%d\t%d\t%s\t%s\t%s\t%d\t%s\t%s",
						en->masterId, en->jobId, en->disponibilidad, 0, estado, nombreNodo,
						nombreCopia, en->numeroBloque, etp, archTemp);
			}

		} else {
			log_info(logger, "   \t%d\t%d\t\t%d\t%s\t%s\t%s\t\t%s\t%s", en->masterId,
					en->jobId, 0, estado, nombreNodo, nombreCopia,
					etp, archTemp);
		}
	}

	list_iterate(tablaEstado, logEn);
	//////////

	t_list * nodos = list_create();
	list_clean(nodos);

	void getNodos(void* entrada){
		EntradaTablaEstado * en = (EntradaTablaEstado *) entrada;

		bool finder(void * nodo) {
			EntradaTablaEstado* node = (EntradaTablaEstado*) nodo;
			return !strcmp(en->nombreNodo, node->nombreNodo);
		}

		if(!list_any_satisfy(nodos, finder)) list_add(nodos, en);
	}
	list_iterate(tablaEstado, getNodos);

	log_info(logger, "Nodo\tCarga\tHistoricas");

	void logNodos(void * nodo){
		EntradaTablaEstado* en = (EntradaTablaEstado*) nodo;
		log_info(logger, "%s\t%d\t%d", en->nombreNodo, trabajoActual(en->nombreNodo), getTareasHistoricas(en->nombreNodo));
	}

	list_iterate(nodos, logNodos);
}

void signal2_handler(int sig){
	if(sig == SIGUSR2){
		logEstadoNodos();
		cabecera = 0;
	}
}

void nuevoCliente(char* remoteHost, int newfd){
	log_info(logger, "Nueva conexion de %s en socket %d", remoteHost, newfd);
	cabecera = 0;
	//Y acá hacer algo con el nuevo cliente conectado
}


void conectarAFileSystem() {
	struct sockaddr_in their_addr; // información de la dirección de destino

	if ((sock_fs = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
	their_addr.sin_port = conexionFileSystem.puerto; // short, Ordenación de bytes de la red
	their_addr.sin_addr.s_addr = inet_addr(conexionFileSystem.ip);
	memset(&(their_addr.sin_zero), 0, 8); // poner a cero el resto de la estructura

	if (connect(sock_fs, (struct sockaddr *) &their_addr,
				sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}
}

void signals() {
	struct sigaction new_action, old_action;

	/* Set up the structure to specify the new action. */
	new_action.sa_handler = recargarConfiguracion;
	new_action.sa_flags = SA_RESTART;
	sigemptyset(&new_action.sa_mask);
	new_action.sa_flags = SA_RESTART;

	sigaction(SIGUSR1, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction(SIGUSR1, &new_action, NULL);

	signal(SIGUSR2, signal2_handler);

	/*

	signal(SIGUSR1, signal_handler);
	*/
}

int main(){

	pthread_mutex_t sem = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&sem);
	signals();
	inicializarYAMA();
	iniciarConexionAFS(&sock_fs);
	log_info(logger, "Para recargar la configuracion utilice el comando kill -USR1 %d", getpid());
	inicializarServer();

	pthread_mutex_lock(&sem);
	return 0;
}
