#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "chat.h"
#include "cliente.h"
#include "dataNode.h"
#include <ifaddrs.h>

int sockfd;

void enviarStructFileSystem(int socket){
	HandshakeNodo handshake;
	handshake.tipomensaje = 2;
	strcpy(handshake.nombreNodo, infoNodo.nombreNodo);

	struct ifaddrs * ifAddrStruct=NULL;
	struct ifaddrs * ifa=NULL;
	void * tmpAddrPtr=NULL;

	getifaddrs(&ifAddrStruct);

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || strcmp("eth0", ifa->ifa_name) != 0) {
			continue;
		}
		if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
			// is a valid IP4 Address
			tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			log_info(logger, "%s IP Address %s\n", ifa->ifa_name, addressBuffer);
			strcpy(handshake.ip, addressBuffer);
		}
	}
	if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);


	handshake.puerto = infoNodo.puertoWorker;
	handshake.bloques = infoNodo.cantidadBloques;

	send(socket, &handshake, sizeof(handshake), 0);
}

int main(int argc, char *argv[]){
	inicializarDataNode();
	iniciarConexionAServer(&sockfd);
	enviarStructFileSystem(sockfd);

	escuchar_chat(sockfd);

	close(sockfd);
	return 0;
}