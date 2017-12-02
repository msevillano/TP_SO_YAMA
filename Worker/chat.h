/*
 * chat.h
 *
 *  Created on: 3/9/2017
 *      Author: utnso
 */

#ifndef CHAT_H_
#define CHAT_H_

//https://es.wikibooks.org/wiki/Programaci%C3%B3n_en_C/Estructuras_y_Uniones
typedef struct {
	int tipoMensaje;
	char mensaje[100];
} __attribute__((packed))
mensajeCorto;

void iniciarConexionAServer(int* sockfd);

#endif /* CHAT_H_ */
