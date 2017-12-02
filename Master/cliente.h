/*
 * cliente.h
 *
 *  Created on: 7/9/2017
 *      Author: utnso
 */

#ifndef CLIENTE_H_
#define CLIENTE_H_

#include <stdint.h>

void iniciarConexionAYAMA(int *socket_yama);
void iniciarConexionANodo(int *socket_nodo, char *ip, uint16_t puerto);

#endif /* CLIENTE_H_ */
