#ifndef CONEXIONESFILESYSTEM_H_
#define CONEXIONESFILESYSTEM_H_

void responderYAMA(int socketYAMA);

enum tipoMensaje {OK = 1, CONEXIONNODO, SOLICITUDARCHIVOYAMA, RECEPCIONBLOQUE, RECEPCIONARCHIVOWORKER, ERRORGUARDARBLOQUE, CONEXIONYAMA=10};
enum tipoMensajeEnviar {GETBLOQUE=1, SETBLOQUE};

#endif /* CONEXIONESFILESYSTEM_H_ */