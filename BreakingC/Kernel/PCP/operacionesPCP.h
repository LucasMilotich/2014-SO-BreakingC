/*
 * operacionesPCP.h
 *
 *  Created on: 13/07/2014
 *      Author: utnso
 */

#ifndef OPERACIONESPCP_H_
#define OPERACIONESPCP_H_
#include "../kernel.h"
#include <stdbool.h>
#include <semaphore.h>
#include <comun/Structs.h>
#include "estructurasPCP.h"

void operacionImprimir(uint32_t ,int32_t ,char*);
void operacionQuantum (t_clienteCPU *, char*);
void operacionError(t_clienteCPU *, char*);
void operacionExit(t_clienteCPU*,char*);
void operacionIO(t_clienteCPU*,char*);
void operacionesConVariablesCompartidas(char,char*,uint32_t);
void operacionesConSemaforos (char,char*,t_clienteCPU *);
void operacionPcbBlocked(t_clienteCPU*, char*);
extern int cpuAcerrar;
extern pthread_mutex_t mutex_exit;
#endif /* OPERACIONESPCP_H_ */
