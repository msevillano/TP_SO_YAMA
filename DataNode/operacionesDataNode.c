#include "operacionesDataNode.h"
#include "dataNode.h"
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>

int getBloque(int bloque, char data[MB]){
    if(bloque < 0){
        return 0;
    }

    int fd = open(infoNodo.rutaDataBin, O_RDONLY);

    if(fd == -1){
        log_error(logger, "No se encontró el data.bin en la ruta %s\n", infoNodo.rutaDataBin);
        return -1;
    }

    mapeoDataBin = mmap(NULL, MB,
                        PROT_READ, MAP_PRIVATE, fd, MB * bloque);

    if(mapeoDataBin == MAP_FAILED){
        log_error(logger, "No se pudo mapear el data.bin en la ruta\n");
        return -1;
    }

    /* copio el contenido del bloque en data; lo copio porque hay que liberar el mapeo y cerrar el archivo */
    strcpy(data, mapeoDataBin);

    munmap(mapeoDataBin, MB); // libero el mapeo
    close(fd);

    return 0;
}

int setBloque(int bloque, char data[MB]){
    int fd = open(infoNodo.rutaDataBin, O_RDWR);

    if(fd == -1){
        log_error(logger, "No se encontró el data.bin en la ruta %s\n", infoNodo.rutaDataBin);
        return -1;
    }

    mapeoDataBin = mmap(NULL, MB,
                        PROT_READ | PROT_WRITE, MAP_SHARED, fd, MB * bloque);

    if(mapeoDataBin == MAP_FAILED){
        log_error(logger, "No se pudo mapear el data.bin en la ruta\n");
        return -1;
    }


    /* copio la data en el mapeo del bloque */
    strcpy(mapeoDataBin, data);

    // guardo el bloque
    if(msync(mapeoDataBin, MB, MS_SYNC) == -1){
        return -1; // error guardando el bloque
    }

    munmap(mapeoDataBin, MB); // libero el mapeo
    close(fd);

    return 0;
}