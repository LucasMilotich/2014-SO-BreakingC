/*
 * estructurasPCP.h
 *
 *  Created on: 13/07/2014
 *      Author: utnso
 */

#ifndef ESTRUCTURASPCP_H_
#define ESTRUCTURASPCP_H_

typedef struct{
	int32_t ID;
	int socket;
	t_pcb* pcbAsignado;
	bool fueAsignado;
	bool programaCerrado;
}t_clienteCPU;

typedef struct{
	char* nombre;
	int32_t valor;
}t_semaforo;
typedef struct{
	char* nombre;
	int32_t valor;
}t_varCompartida;

typedef struct{
	t_pcb* pcb;
	char* nombreSemaforo;
}t_pcbBlockedSemaforo;

typedef struct{
	t_pcb* pcb;
	int32_t valor;
}t_solicitudes;

typedef struct{
	int32_t ID_IO;
	char* nombre;
	int32_t valor;
	sem_t semaforo;
	t_queue *colaSolicitudes; 		//va a ser una cola de t_solicitudes
}t_IO;

#endif /* ESTRUCTURASPCP_H_ */
