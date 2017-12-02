#include "yama.h"

#ifndef TP_2017_2C_LANZALLAMAS_PLANIFICACIONYAMA_H
#define TP_2017_2C_LANZALLAMAS_PLANIFICACIONYAMA_H


typedef struct{
	char nombreNodo[100];
	int numBloque;
	int bytes;
	char nombreTemp[255];
}__attribute__((packed))
transfError;

int getTareasHistoricas(char * nombreNodo);
int trabajoActual(char* nombreNodo);
void ordenarNodos(t_list * nodos);
int estaEnNodo(DescriptorBloque bloque, InfoNodo *pClock);
void planificarBloquesYEnviarAMaster(int socket_master, int bloques, t_list * listaNodos);
void generarArchivoTemporal(char * nombre, char * file);
void replanificar(int socket);
void logEntrada(int masterId, int jobId, int disp, int trabajo, char*estado,
		char* nombreNodo, char*nombreCopia, int numBloque, char* etapa,
		char* archTemp);

#endif //TP_2017_2C_LANZALLAMAS_PLANIFICACIONYAMA_H
