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
#include "mainFS.h"
#include "serializacionFileSystem.h"
#include "fileSystem.h"
#include "estructurasFileSystem.h"
#include <sys/ioctl.h>
#include <errno.h>

int listener;    // descriptor de socket a la escucha
int fdmax;       // número máximo de descriptores de fichero
fd_set read_fds; // conjunto temporal de descriptores de fichero para select()

void comprobarConexion(int numbytes, int socket){
    if (numbytes <= 0) {
        // error o conexión cerrada por el cliente
        if (numbytes == 0) {
            // conexión cerrada
            log_debug(logger, "selectserver: socket %d hung up\n", socket);

            marcarNodoDesconectado(socket);
        } else {
            perror("recv");
        }
        close(socket); // bye!
        FD_CLR(socket, &master); // eliminar del conjunto maestro
    }
}

void manejarCliente(int newfd){
    int numbytes;
    int buf;

    buf = 0;

    int count;
    ioctl(newfd, FIONREAD, &count);

	if (count == 0) {
        int flag = 1123;
		if (send(newfd, &flag, sizeof(int), MSG_NOSIGNAL) < 0) {
			log_debug(logger, "selectserver: socket %d hung up\n", newfd);
			marcarNodoDesconectado(newfd);
			close(newfd); // bye!
			FD_CLR(newfd, &master); // eliminar del conjunto maestro
		}
    	return;
    }

    numbytes = recv(newfd, &buf, sizeof(int), 0); //leo el primer byte. Me dirá el tipo de paquete. (es un int)

    comprobarConexion(numbytes, newfd); //Me fijo si lo que recibí esta todo ok.

    manejarDatos(buf, newfd); //Si llegamos hasta acá manejamos los datos que recibimos.
}

void gestionarNuevaConexion(){
    // gestionar nuevas conexiones

    int newfd;        // descriptor de socket de nueva conexión aceptada
    struct sockaddr_in remoteaddr; // dirección del cliente
    int addrlen;

    addrlen = sizeof(remoteaddr);

    if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr,
                        &addrlen)) == -1) {
        perror("accept");
        exit(1);
    }

    else {
        FD_SET(newfd, &master); // añadir al conjunto maestro
        if (newfd > fdmax) {    // actualizar el máximo
            fdmax = newfd;
        }

        nuevoCliente(inet_ntoa(remoteaddr.sin_addr), newfd);

        /*Esto sirve para crear un hilo por cada conexión. Por el momento no lo necesitamos.
        int rc;
        pthread_t tid[MAXCLIENTES];
        rc = pthread_create(&tid[contador], NULL, manejarCliente, newfd);
                if(rc) printf("no pudo crear el hilo");
        contador++;
        */
        //verificar contadorde hilos (para cuando cerramos hilos)
    }
}

void doSelect(){

    FD_ZERO(&read_fds);

    //Bucle Principal
    for(;;) {

        read_fds = master; // cópialo

        if (select((fdmax)+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        // explorar conexiones existentes en busca de datos que leer
        int actualfd;
        for(actualfd = 0; actualfd <= fdmax; actualfd++) {

            if (FD_ISSET(actualfd, &read_fds)) { // ¡¡tenemos datos!!
                pthread_mutex_lock(&semaforoConsola);
                if (actualfd == listener) gestionarNuevaConexion();

                else manejarCliente(actualfd);
                pthread_mutex_unlock(&semaforoConsola);

            }
        }
    }
}


void setListener(){
    // obtener socket a la escucha
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    // obviar el mensaje "address already in use" (la dirección ya se está usando)
    int yes=1;        // para setsockopt() SO_REUSEADDR, más abajo
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,
                   sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    // enlazar
    struct sockaddr_in myaddr;     // dirección del servidor
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = htons(config_get_int_value(config, PUERTO_FILESYSTEM));
    memset(&(myaddr.sin_zero), '\0', 8);
    if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
        perror("bind");
        exit(1);
    }
    // escuchar
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(1);
    }
}

void * iniciarServer(){

    FD_ZERO(&master);    		// borra los conjuntos maestro y temporal

    setListener();

    FD_SET(listener, &master);	// añadir listener al conjunto maestro

    // seguir la pista del descriptor de fichero mayor
    fdmax = listener; 			// por ahora es éste

    doSelect();// bucle principal

    return NULL;
}

void inicializarServer(){
    iniciarServer();
}
