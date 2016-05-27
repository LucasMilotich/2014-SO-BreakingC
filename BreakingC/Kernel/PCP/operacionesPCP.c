/*
 * operacionesPCP.c
 *
 *  Created on: 13/07/2014
 *      Author: Nicolas Garcia
 */
#include "operacionesPCP.h"
#include  <comun/Structs.h>
pthread_mutex_t mutex;
t_config *config;
t_list * listaClientes, *listaVarCompartidas, *listaSemaforos, *listaIO, *listaBloqueados;
t_log* log_pcp;
sem_t semClienteOcioso;
int cpuAcerrar;


void operacionImprimir(uint32_t tipoOperacion, int32_t pcbIDasignado,
		char*buffer) {
	t_imprimibles *imprimible = malloc(sizeof(t_imprimibles));
	imprimible->tipoDeValor = tipoOperacion;
	imprimible->valor = buffer;
	imprimible->PCB_ID = pcbIDasignado;
	queue_push(colaImprimibles, imprimible);
	sem_post(&semMensajeImpresion);
}

void operacionesConSemaforos(char operacion, char* buffer, t_clienteCPU *unCliente) {
	log_debug(log_pcp, "Se ingresa a operacionConSemaforos");
	_Bool _obtenerSemaforo(t_semaforo *unSemaforo) {
		return (strcmp(unSemaforo->nombre, buffer) == 0);
	}

	t_semaforo * semaforo;
	t_pcbBlockedSemaforo * unPcbBlocked;
	char* bufferPCB;
	semaforo = list_find(listaSemaforos, (void*) _obtenerSemaforo);
	//free(buffer);
	switch (operacion) {
	case WAIT:
		if (unCliente->programaCerrado)
		{
			enviarDatos(unCliente->socket, &buffer, sizeof(buffer), ERROR);
			break;
		}

		if (semaforo->valor > 0) {
			//No se bloquea
			enviarDatos(unCliente->socket, &buffer, sizeof(buffer), NOTHING);
		} else {
			//Se bloquea
			enviarDatos(unCliente->socket, &buffer, sizeof(buffer), WAIT);
			unPcbBlocked = malloc(sizeof(t_pcbBlockedSemaforo));
			recibirDatos(unCliente->socket, &bufferPCB, NULL );
			unPcbBlocked->nombreSemaforo = semaforo->nombre;
			unPcbBlocked->pcb = deserializar_pcb((char **) &bufferPCB);
			free(bufferPCB);
			list_add(listaBloqueados, unPcbBlocked);
			log_debug(log_pcp,
					"Agregado a la colaBlockeadoPorSemaforo el PCB del pid: %d",
					unPcbBlocked->pcb->pcb_id);
			unCliente->fueAsignado = false;
			//unCliente->pcbAsignado.pcb_id=-1;
			sem_post(&semClienteOcioso);
		}
		semaforo->valor--;
		free(buffer);
		break;
	case SIGNAL:

		//comprobar si podria despertar a alguien
		if (semaforo->valor < 0) {

			_Bool _obtenerPcbBlockeado(t_pcbBlockedSemaforo *unPcbBlockeado) {
				return (strcmp(unPcbBlockeado->nombreSemaforo, semaforo->nombre)
						== 0);
			}
			t_pcbBlockedSemaforo* pcbBlockeado;
			pcbBlockeado = list_find(listaBloqueados,
					(void*) _obtenerPcbBlockeado);
			//Compruebo si hay algun pcb blocked por este semaforo
			if (pcbBlockeado != NULL ) {
				//lo desbloqueo, lo mando a la colaReady

				_Bool _quitarPcbBlockeado(t_pcbBlockedSemaforo *unPcbBlockeado) {
					return (unPcbBlockeado->pcb->pcb_id
							== pcbBlockeado->pcb->pcb_id);
				}
				pthread_mutex_lock(&mutex);
				list_remove_by_condition(listaBloqueados,
						(void*) _quitarPcbBlockeado);
				queue_push(colaReady, pcbBlockeado->pcb);
				sem_post(&semNuevoPcbColaReady);
				pthread_mutex_unlock(&mutex);
			}
		}
		semaforo->valor++;
		break;
	}
}

void operacionPcbBlocked(t_clienteCPU*unCliente, char*buffer) {
	log_debug(log_pcp, "Se ingresa a operacionPcbBlocked");
	//Voy a actualizar el pcb que habia puesto en la cola de blocked
	t_pcb * unPCB;
	t_pcbBlockedSemaforo * unPcbBlocked;

	_Bool _obtenerPcbBlockeado(t_pcbBlockedSemaforo *unPcbBlockeado) {
		return (unPcbBlockeado->pcb->pcb_id == unPCB->pcb_id);
	}

	log_debug(log_pcp,
			"ClienteCPU %d envía unPCB para la lista de bloqueados(Actualizar)",
			unCliente->ID);

	unPCB = deserializar_pcb((char **) &buffer);
	unPcbBlocked = list_find(listaBloqueados, (void*) _obtenerPcbBlockeado);
	//iria un free aca
	unPcbBlocked->pcb = unPCB;

	log_debug(log_pcp,
			"Agregado a la colaBlockeadoPorSemaforo el PCB del pid: %d",
			unPCB->pcb_id);
	unCliente->fueAsignado = false;
	//unCliente->pcbAsignado.pcb_id=-1;
	sem_post(&semClienteOcioso);
}

void operacionesConVariablesCompartidas(char operacion, char *buffer,
		uint32_t socketCliente) {
	log_debug(log_pcp, "Se ingresa a operacionVariablesCompartidas");
	t_op_varCompartida * varCompartida;
	t_varCompartida * varCompartidaEnLaLista;
	char* valorEnBuffer, *bufferLeerVarCompartida;
	_Bool _obtenerVariable(t_varCompartida * unaVariable) {
		return (strcmp(unaVariable->nombre, varCompartida->nombre) == 0);
	}
	_Bool _obtenerVariable2(t_varCompartida * unaVariable) {
		return (strcmp(unaVariable->nombre, bufferLeerVarCompartida) == 0);
	}

	// Dependiendo de la operacion..
	switch (operacion) {
	case ASIG_VAR_COMPARTIDA:
		log_debug(log_pcp,
				"Cliente quiere asignar valor a variable compartida");
		//Asigno el valor a la variable
		varCompartida = deserializar_opVarCompartida(&buffer);
		varCompartidaEnLaLista = list_find(listaVarCompartidas,
				(void*) _obtenerVariable);
		varCompartidaEnLaLista->valor = varCompartida->valor;
		free(varCompartida);
		break;
	case LEER_VAR_COMPARTIDA:
		log_debug(log_pcp, "Cliente quiere leer variable compartida");
		//Leo el valor de la variable y lo envio
		valorEnBuffer = malloc(sizeof(uint32_t));
		//	bufferLeerVarCompartida=malloc(strlen(buffer));
		bufferLeerVarCompartida = buffer;
		varCompartidaEnLaLista = list_find(listaVarCompartidas,
				(void*) _obtenerVariable2);
		memcpy(valorEnBuffer, &(varCompartidaEnLaLista->valor),
				sizeof(uint32_t));
		enviarDatos(socketCliente, &valorEnBuffer, sizeof(uint32_t), NOTHING);
		free(valorEnBuffer);
		free(bufferLeerVarCompartida);
		break;
	}
}

void operacionQuantum(t_clienteCPU *unCliente, char*buffer) {
	log_debug(log_pcp, "Se ingresa a operacionQuantum");
	t_pcb * unPCB ;//= malloc(sizeof(t_pcb));
	log_debug(log_pcp, "Cliente %d envía unPCB", unCliente->ID);
	unPCB = deserializar_pcb((char **) &buffer);

	pthread_mutex_lock(&mutex);
	queue_push(colaReady, (void*) unPCB);
	pthread_mutex_unlock(&mutex);
	log_debug(log_pcp, "Agregado a la colaReady el PCB del pid: %d",
			unPCB->pcb_id);
	if (cpuAcerrar!=unCliente->socket) {
		unCliente->fueAsignado = false;
		sem_post(&semClienteOcioso);
	}
	sem_post(&semNuevoPcbColaReady);

}

void operacionIO(t_clienteCPU *unCliente, char*buffer) {
	log_debug(log_pcp, "Se ingresa a operacionIO");
	t_op_IO *opIO;
	t_pcb *elPCB;
	t_solicitudes *solicitud = malloc(sizeof(t_solicitudes));

	log_debug(log_pcp, "Cliente %d envía unPCB", unCliente->ID);
	opIO = deserializar_opIO(&buffer);
	free(buffer);
	recibirDatos(unCliente->socket, &buffer, NULL);
	elPCB = deserializar_pcb((char **) &buffer);
	solicitud->pcb = elPCB;
	solicitud->valor = opIO->tiempo;

	bool _BuscarIO(t_IO* element) {
		return (strcmp(opIO->dispositivo, element->nombre) == 0);
	}
	t_IO * entradaSalida;
	entradaSalida = list_find(listaIO, (void*) _BuscarIO);
	if (entradaSalida == NULL ) {
		log_error(log_pcp,
				"Dispositivo IO no encontrado en archivo de configuracion");
		log_debug(log_pcp, "Agregado el PCB_ID:%d a la cola Ready",
				elPCB->pcb_id);
		queue_push(colaReady, elPCB);
		sem_post(&semNuevoPcbColaReady);
	} else {
		queue_push(entradaSalida->colaSolicitudes, solicitud);
		sem_post(&entradaSalida->semaforo);
	}
	if (cpuAcerrar!=unCliente->socket) {
			unCliente->fueAsignado = false;
			sem_post(&semClienteOcioso);
		}
}

void operacionExit(t_clienteCPU *unCliente, char*buffer) {
	log_debug(log_pcp, "Se ingresa a operacionExit");
	t_pcb * unPCB;
	log_debug(log_pcp, "CPU %d envía unPCB para desalojar", unCliente->ID);
	unPCB = deserializar_pcb((char **) &buffer);
	char * texto = "Se ha finalizado el programa correctamente";
	operacionImprimir(EXIT, unPCB->pcb_id, texto);
	log_debug(log_pcp, "Agregado a la colaExit el PCB del pid: %d",
			unPCB->pcb_id);
	queue_push(colaExit, (void*) unPCB);
	sem_post(&semProgExit);
	//pthread_mutex_unlock(&mutex_exit);
	if (cpuAcerrar!=unCliente->socket) {
			unCliente->fueAsignado = false;
			sem_post(&semClienteOcioso);
		}
}

void operacionError(t_clienteCPU *unCliente, char*buffer) {
	log_debug(log_pcp, "Se ingresa a operacionError");
	t_pcb * unPCB;
	log_debug(log_pcp, "Cliente %d envía unPCB", unCliente->ID);
	unPCB = deserializar_pcb((char **) &buffer);
	char * texto = "Se ha detenido la ejecucion por seg.Fault";
	operacionImprimir(ERROR, unPCB->pcb_id, texto);
	log_debug(log_pcp, "Agregado a la colaExit(Por error) el PCB del pid: %d",
			unPCB->pcb_id);
	queue_push(colaExit, unPCB);
	sem_post(&semProgExit);
	if (cpuAcerrar!=unCliente->socket) {
			unCliente->fueAsignado = false;
			sem_post(&semClienteOcioso);
		}
}
