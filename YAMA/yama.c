#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>
#include "yama.h"

void _cargarConfiguracion(void);
void _copiarFileSystemIP(void);
void _definirAlgoritmoBalanceo(void);
void _crearLogger(void);
void _logConfig(void);

void inicializarYAMA(){
	_crearLogger();
	_cargarConfiguracion();
	_logConfig();
	tablaEstado = list_create();
	cabecera = 0;
}

void recargarConfiguracion(){
	// destruyo la config original ya que el config_create hace un malloc
	config_destroy(config);
	_cargarConfiguracion();
	log_info(logger, "Recarga de configurcion OK\n");
	cabecera = 0;
}

void intToString(int numero, char * string){
	sprintf(string, "%d", numero);
}

void _cargarConfiguracion(){
	config = config_create(ARCHIVO_CONFIGURACION);
	if (!config_has_property(config, FS_IP) ||
		!config_has_property(config, FS_PUERTO) ||
		!config_has_property(config, RETARDO_PLANIFICACION) ||
		!config_has_property(config, ALGORITMO_BALANCEO) ||
		!config_has_property(config, DISPONIBILIDAD_BASE)){
		log_error(logger, "Bad Config");
		exit(120);
	}

	conexionFileSystem.puerto = htons(config_get_int_value(config, FS_PUERTO));
	retardoPlanificacion = config_get_int_value(config, RETARDO_PLANIFICACION);
	_definirAlgoritmoBalanceo();
	_copiarFileSystemIP();
	disponibilidad_base = config_get_int_value(config, DISPONIBILIDAD_BASE);
}

void _copiarFileSystemIP(){
	char * ip;

	ip = config_get_string_value(config, FS_IP);

	int i = 0;

	// Copio el valor caracter a caracter porque ip es un char[20] y la config devuelve char *
	while(*ip != '\0'){
		conexionFileSystem.ip[i] = *ip;
		ip++;
		i++;
	}
}

void _definirAlgoritmoBalanceo(){
	char * algoritmo;

	algoritmo = config_get_string_value(config, ALGORITMO_BALANCEO);

	if(strcmp(algoritmo,CLOCK_CONFIG) == 0){
		algoritmoBalanceo = CLOCK;
	} else if(strcmp(algoritmo,WEIGHTED_CLOCK) == 0){
		algoritmoBalanceo = WEIGHTEDCLOCK;
	} else {
		printf("ALGORITMO DE PLANIFICACION DESCONOCIDO\n");
		exit(121);
	}
}

void _crearLogger(){
	logger = log_create(ARCHIVO_LOGGER, MODULO, true, LOG_LEVEL_INFO);
}

void _logConfig(){
	log_debug(logger, "Config:\nFS_IP: %s\nFS_PUERTO: %d\nRETARDO_PLANIFICACION: %d\nALGORITMO_BALANCEO: %s",
			  config_get_string_value(config, FS_IP),
			  config_get_int_value(config, FS_PUERTO),
			  config_get_int_value(config, RETARDO_PLANIFICACION),
			  config_get_string_value(config, ALGORITMO_BALANCEO));
}
