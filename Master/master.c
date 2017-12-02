/*
 * master.c
 *
 *  Created on: 10/9/2017
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>
#include "master.h"

void _cargarConfiguracion();
void _copiarYAMAIP();
void _crearLogger();
void _logConfig();

void inicializarMaster(){
	_crearLogger();
	_cargarConfiguracion();
	_logConfig();
}

void _cargarConfiguracion(){
	config = config_create(ARCHIVOCFG);
	if (!config_has_property(config, YAMA_IP) || !config_has_property(config, YAMA_PUERTO)){
		log_error(logger, "Bad Config");
		exit(120);
	}

	conexionYAMA.puerto = htons(config_get_int_value(config, YAMA_PUERTO));
	_copiarYAMAIP();
}

void _copiarYAMAIP(){
	char * ip;

	ip = config_get_string_value(config, YAMA_IP);

	int i = 0;

	// Copio el valor caracter a caracter porque ip es un char[20] y la config devuelve char *
	while(*ip != '\0'){
		conexionYAMA.ip[i] = *ip;
		ip++;
		i++;
	}
}

void _crearLogger(){
	logger = log_create(ARCHIVO_LOGGER, MODULO, true, LOG_LEVEL_INFO);
}

void _logConfig(){
	log_debug(logger, "Config:\nYAMA_IP: %s\nYAMA_PUERTO: %d",
			config_get_string_value(config, YAMA_IP),
			config_get_int_value(config, YAMA_PUERTO));
}
