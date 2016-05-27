/*
 * Franco Bustos
 *
 * Ejemplo de como un servidor puede manejar varios clientes con select().
 * Este programa hace de cliente de dicho servidor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sockets/Socket.h>
#include <sockets/Socket_Cliente.h>
#include <unistd.h>
#include <comun/Structs.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include "comun/UMV_Structs.h"
#include <commons/string.h>
//#include <stdbool.h>

/* Programa principal. Abre la conexión, recibe quantum y pcb */

#define TAMANIO_VARIABLE 5
extern char* etiquetas;
extern t_log* Log;
extern t_config* config;
extern t_pcb* pcb;
extern int socket_PCP;
extern int socket_UMV;
extern char operacion;
extern t_dictionary* dic_variables;
extern bool etiq_creadas;
#define SIZE_IC 8 //tamanio de indice de codigo en memoria

int ConectarPCP() {
	char *direccion = config_get_string_value(config, "IP_KERNEL");
	int puerto = config_get_int_value(config, "PUERTO_KERNEL");

	int sock; /* descriptor de conexión con el servidor */

	log_debug(Log, "Conectando con el PCP");
	/* Se abre una conexión con el servidor */
	sock = Abrir_Conexion(direccion, puerto);
	if (sock < 0) {
		log_error(Log, "Ocurrio un error al conectarse con el PCP");
		return -1;
	}

	log_debug(Log, "Conectado con el PCP");
	return sock;
}

int ConectarUMV() {
	char *direccion = config_get_string_value(config, "IP_UMV");
	int puerto = config_get_int_value(config, "PUERTO_UMV");

	int sock; /* descriptor de conexión con el servidor */

	log_debug(Log, "Conectando con la UMV");
	/* Se abre una conexión con el servidor */
	sock = Abrir_Conexion(direccion, puerto);
	if (sock < 0) {
		log_error(Log, "Ocurrio un error al conectarse con la UMV");
		return -1;
	}

	log_debug(Log, "Conectado con la UMV");
	return sock;
}

int RecibirPCB(t_pcb** pcb) {
	char* datos;
	log_debug(Log, "Esperando PCB..");
	/* Si ha habido error de lectura lo indicamos y salimos */
	if (recibirDatos(socket_PCP, &datos, NULL ) < 1) {
		log_error(Log, "Error al recibir el PCB");
		return -1;
	}
	log_debug(Log, "PCB recibido");
	log_debug(Log, "Deserializando..");
	*pcb = deserializar_pcb(&datos);
	log_debug(Log, "PCB deserializado");
	log_trace(Log, "PC = %u", (*pcb)->PC);
	log_trace(Log, "cursor_stack = %u", (*pcb)->cursor_stack);
	log_trace(Log, "index_codigo = %u", (*pcb)->index_codigo);
	log_trace(Log, "index_etiq = %u", (*pcb)->index_etiq);
	log_trace(Log, "pcb_id = %u", (*pcb)->pcb_id);
	log_trace(Log, "peso = %u", (*pcb)->peso);
	log_trace(Log, "seg_codigo = %u", (*pcb)->seg_codigo);
	log_trace(Log, "seg_stack = %u", (*pcb)->seg_stack);
	log_trace(Log, "tamanio_contexto = %u", (*pcb)->tamanio_contexto);
	log_trace(Log, "tamanio_index_etiq = %u", (*pcb)->tamanio_index_etiq);
	free(datos);
	return EXIT_SUCCESS;
}

t_EstructuraInicial* RecibirQuantum() {
	t_EstructuraInicial* est_quantum;
	char* datos;
	log_debug(Log, "Esperando Quantum..");
	if (recibirDatos(socket_PCP, &datos, NULL ) <= 0) {
		log_error(Log, "Error al recibir el quantum");
		return NULL ;
	}
	//memcpy(&quantum, datos, sizeof(uint32_t));
	est_quantum = deserializar_EstructuraInicial(&datos);
	log_debug(Log, "El quantum recibido es: %d", est_quantum->Quantum);
	free(datos);
	return est_quantum;
}

char* ObtenerSentencia(t_pcb* pcb) {
	char** arrayStrings;
	char* respuesta;
	uint32_t offset = SIZE_IC * (pcb->PC - 1);
	if (offset < 0) {
		log_error(Log, "El Program Counter (PC) debe ser mayor que 0");
		return NULL ;
	}
	log_debug(Log, "Obteniendo la proxima sentencia a ejecutar");
	t_solicitarBytes* solicitar = malloc(sizeof(t_solicitarBytes));
	/* Solicito base y tamanio de la sentencia
	 * por el indice de codigo y PC
	 */
	solicitar->base = pcb->index_codigo;
	solicitar->offset = offset;
	solicitar->tamanio = SIZE_IC;
	respuesta = enviarOperacion(SOLICITAR_BYTES, solicitar, socket_UMV);
	if (respuesta[0] == -1){
		operacion = ERROR;
		free(respuesta);
		free(solicitar);
		return NULL;
	}
	/*
	 * Solicito la sentencia
	 */
	solicitar->base = pcb->seg_codigo;
	memcpy(&solicitar->offset, respuesta, 4);
	memcpy(&solicitar->tamanio, respuesta + 4, 4);

	free(respuesta);
	respuesta = enviarOperacion(SOLICITAR_BYTES, solicitar, socket_UMV);
	if (respuesta[0] == -1){
		operacion = ERROR;
		free(respuesta);
		free(solicitar);
		return NULL;
	}
	/*
	 * Se sacan los \n de la sentencia
	 */
	arrayStrings = string_split(respuesta, "\n");
	strcpy(respuesta, arrayStrings[0]);

	void _destroyStrings(char* str){
		free(str);
	}

	string_iterate_lines(arrayStrings, _destroyStrings);
	free(arrayStrings);
	free(solicitar);
	return respuesta;
}

bool ObtenerDiccionarioEtiquetas() {
	if (pcb->tamanio_index_etiq > 0)
	{
		t_solicitarBytes* solicitar = malloc(sizeof(t_solicitarBytes));
		solicitar->base = pcb->index_etiq;
		solicitar->offset = 0;
		solicitar->tamanio = pcb->tamanio_index_etiq;
		etiquetas = enviarOperacion(SOLICITAR_BYTES, solicitar, socket_UMV);
		if (etiquetas[0] == -1){
			operacion = ERROR;
		}
		free(solicitar);
		return true;
	}
	return false;
}

void HandshakeUMV() {
	t_handshake* handshake = malloc(sizeof(t_handshake));
	handshake->tipo = 1;
	enviarOperacion(HANDSHAKE, handshake, socket_UMV);
	free(handshake);
}

void CambiarProcesoActivo() {
	t_cambio_proc_activo* proc_activo = malloc(sizeof(proc_activo));
	proc_activo->programID = pcb->pcb_id;
	enviarOperacion(CAMBIO_PROC_ACTIVO, proc_activo, socket_UMV);
	free(proc_activo);
}

void CambioContexto(){
	log_debug(Log, "El proceso necesita ser desalojado, debido a que:");

	switch (operacion) {
		case QUANTUM:
			log_debug(Log, "Finalizo la rafaga de Quantum");
			break;
		case WAIT:
			log_debug(Log, "El proceso quedo bloqueado por WAIT de un semaforo");
			break;
		case IO:
			log_debug(Log, "El proceso quedo bloqueado por ENTRADA/SALIDA");
			break;
		case EXIT:
			log_debug(Log, "Se ha terminado de ejecutar el programa");
			break;
		default:
			log_debug(Log, "Ocurrio un error");
			break;
	}
	if (etiq_creadas)
		free(etiquetas);
	dictionary_destroy_and_destroy_elements(dic_variables, (void*) variable_destroy);
	char *buffer = serializar_pcb(pcb);
	enviarDatos(socket_PCP, &buffer, sizeof(t_pcb), operacion);
	free(buffer);
	free(pcb);
	return;
}
