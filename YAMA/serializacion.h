
#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include "yama.h"

enum tipoProceso {YAMA, FILESYSTEM, DATANODE, MASTER, WORKER};

void manejarDatos(int buf, int socket);
void enviarSolicitudReduccion(int socket, t_list * transformacionesRealizadas);
int desasociarSocketJob(int socket);



#endif /* SERIALIZACION_H_ */
