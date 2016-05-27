/*
 * servidor.h
 *
 *  Created on: 29/04/2014
 *      Author: utnso
 */

#ifndef SERVIDOR_H_
#define SERVIDOR_H_



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdint.h>
#include <netinet/in.h>
#include <pthread.h>
#include "estructuras.h"


//#define PUERTO "6667"
#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo


void *connection_handler(void *);
t_solicitarBytes* deserializarSolicitarBytes(char** );
t_enviarBytes* deserializarEnviarBytes(char** );
t_handshake* deserializarHandshake(char** );
t_crear_segmento* deserializarCrearSegmento(char** );
t_destruir_segmento* deserializarDestruirSegmento(char** );
t_cambio_proc_activo* deserializarCambiarProcesoActivo(char**);
int crearSegmento(t_crear_segmento*);




#endif /* SERVIDOR_H_ */
