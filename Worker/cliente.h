#ifndef CLIENTE_H_
#define CLIENTE_H_
void iniciarConexionAServer(int* sockfd);

void conectarANodo(int* sockfd, char*ip, uint16_t puerto);

#endif /* CLIENTE_H_ */
