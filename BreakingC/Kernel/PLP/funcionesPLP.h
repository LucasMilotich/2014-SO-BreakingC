/*
 * funcionesPLP.h
 *
 *  Created on: 19/06/2014
 *      Author: utnso
 */

#ifndef FUNCIONESPLP_H_
#define FUNCIONESPLP_H_
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <parser/metadata_program.h>
#include <stdint.h>
#include <comun/UMV_Structs.h>
#include <comun/Structs.h>
#include "plp.h"
#include "../kernel.h"
#include <semaphore.h>
#include <pthread.h>

extern t_log* logPlp;
extern t_list* listaClientesProgramas;
extern pthread_mutex_t mutex_exit;

char* serializadoIndiceDeCodigo(t_size , t_intructions*); /* Proc. auxiliar para deserializar el mapa de instrucciones de codigo*/
int procDestruirSeg(int32_t,int); /* Recibe el id del programa y el descriptor socket. Manda a borrar a la UMV los segmentos creados para
cierto pcb*/
void* vaciarColaExit(void*); /*Vacia la cola exit cada vez que llegue un pcb a dicha cola*/
int crearConexionConUMVYConectar(); /* Crea un socket con la info del servidor UMV*/
bool _menorPeso(t_pcb*,t_pcb*);/* Funcion comparadora, utilizada para el algoritmo SJN*/
t_pcb* crearPCB(char*,int);/* Recibe el codigo del script AnSisOp, crea los segmentos correspondientes e inicializa la estructura pcb*/


#endif /* FUNCIONESPLP_H_ */
