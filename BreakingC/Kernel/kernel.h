/*
 * kernel.h
 *
 *  Created on: 20/05/2014
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <commons/log.h>
#include <sockets/Socket_Servidor.h>
#include <sockets/Socket.h>
#include <comun/Structs.h>

//#define RUTA_ARCHIVO_CONFIGURACION 	"/home/utnso/tp-2014-1c-breaking-c/BreakingC/Kernel/Config_Kernel.txt"
//#define RUTA_ARCHIVO_LOG_KERNEL		"/home/utnso/tp-2014-1c-breaking-c/BreakingC/Kernel/log_kernel.txt"


typedef struct{
	char* valor;
	uint32_t PCB_ID;
	uint32_t tipoDeValor;
}t_imprimibles;

extern t_queue * colaImprimibles;

extern  t_list*  colaNew;
extern  t_queue* colaReady;
extern  t_queue* colaExit;
extern t_list* listaBloqueados;

extern  uint32_t nroProg; //id programa
extern  int multiProg;

extern  sem_t semMultiProg;
extern  sem_t semNuevoProg;
extern  sem_t semProgExit;
extern sem_t semNuevoPcbColaReady;
extern sem_t semMensajeImpresion;
extern sem_t semProgramaFinaliza;

extern int32_t pcbAFinalizar;

extern t_config* config;
extern t_log *log_kernel;

extern pthread_mutex_t mutex_exit;

void *comienzaPLP(void); /* Comienza el hilo PLP*/
void *comienzaPCP(void); /* Comienza el hilo PLP*/
t_log* creacionLog(char*,char*);


#endif /* KERNEL_H_ */
