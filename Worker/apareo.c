#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apareo.h"

void apareo(char * rutaArchivo1, char * rutaArchivo2, char * rutaArchivoDefinitivo){
	char * lineaFile1 = NULL, * lineaFile2 = NULL;
	size_t longitudLineaFile1 = 255, longitudLineaFile2 = 255;
	ssize_t caracteresLeidosFile1, caracteresLeidosFile2;
	FILE * archivo1, * archivo2, * archivoDefinitivo;

	lineaFile1 = (char *)malloc(longitudLineaFile1 * sizeof(char));
	lineaFile2 = (char *)malloc(longitudLineaFile2 * sizeof(char));

	archivo1 = fopen(rutaArchivo1, "r");
	archivo2 = fopen(rutaArchivo2, "r");
	archivoDefinitivo = fopen(rutaArchivoDefinitivo, "w");

	caracteresLeidosFile1 = getline(&lineaFile1, &longitudLineaFile1, archivo1);
	caracteresLeidosFile2 = getline(&lineaFile2, &longitudLineaFile2, archivo2);

	while(caracteresLeidosFile1 > -1 && caracteresLeidosFile2 > -1){
		if(strcmp(lineaFile1, lineaFile2) <= 0){
			fwrite(lineaFile1, 1, caracteresLeidosFile1, archivoDefinitivo);
			caracteresLeidosFile1 = getline(&lineaFile1, &longitudLineaFile1, archivo1);
		}else{
			fwrite(lineaFile2, 1, caracteresLeidosFile2, archivoDefinitivo);
			caracteresLeidosFile2 = getline(&lineaFile2, &longitudLineaFile2, archivo2);	
		}
	}

	while(caracteresLeidosFile1 > -1){
		fwrite(lineaFile1, 1, caracteresLeidosFile1, archivoDefinitivo);
		caracteresLeidosFile1 = getline(&lineaFile1, &longitudLineaFile1, archivo1);
	}

	while(caracteresLeidosFile2 > -1){
		fwrite(lineaFile2, 1, caracteresLeidosFile2, archivoDefinitivo);
		caracteresLeidosFile2 = getline(&lineaFile2, &longitudLineaFile2, archivo2);	
	}	

	fclose(archivo1);
	fclose(archivo2);
	fclose(archivoDefinitivo);
}
