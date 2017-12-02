#ifndef FUNCIONESCONSOLAFS_H_
#define FUNCIONESCONSOLAFS_H_


int yama_mkdir(char* path_dir);

#define TEMPFILE "tempFile"

//FUNCIONES AUX
char *yamaGetNFile(char *path);
char *yamaGetPathPadre(char *path);
Directorio yamaCrearDirectorio(int id, char* nombre, int id_padre);
void asignarEspacioEnTabla(Directorio newDir);
void hiloConsola();
void crear_hilo_consola();
int cpfrom(char * archivo, char * archivoFS);
void md5Consola(char * archivo);
void rmArchivo(char * path);

void formatFileSystem();

#endif /* FUNCIONESCONSOLAFS_H_ */
