/*
 * Structs.h
 *
 *  Created on: 01/06/2014
 *      Author: fbustos
 */

#ifndef STRUCTS_H_
#define STRUCTS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "../sockets/Socket.h"

enum OPERACIONES_GENERALES {
	ERROR = -1, NOTHING, SUCCESS
};

enum CPU_PCP {
	QUANTUM = 2,
	IO,
	EXIT,
	IMPRIMIR_VALOR,
	IMPRIMIR_TEXTO,
	LEER_VAR_COMPARTIDA,
	ASIG_VAR_COMPARTIDA,
	WAIT,
	SIGNAL,
	SIGUSR
};

typedef struct op_varCompartida {
	char* nombre;
	uint32_t longNombre;
	uint32_t valor;
} t_op_varCompartida;

typedef struct op_IO {
	char* dispositivo;
	uint32_t tiempo;
} t_op_IO;

typedef struct pcb {
	int32_t pcb_id;
	uint32_t seg_codigo;
	uint32_t seg_stack;
	uint32_t cursor_stack;
	uint32_t index_codigo;
	uint32_t index_etiq;
	uint32_t PC;
	uint32_t tamanio_contexto;
	uint32_t tamanio_index_etiq;
	uint32_t peso;
} t_pcb;

typedef struct estructuraInicial {
	uint32_t Quantum;
	uint32_t RetardoQuantum;
} t_EstructuraInicial;

typedef struct {
	char *nombre;
	uint32_t offset;
} t_variable;

t_variable *variable_create(char *nombre, uint32_t offset);
void variable_destroy(t_variable *self);

t_pcb* deserializar_pcb(char**);
char* serializar_pcb(t_pcb*);

char * serializar_EstructuraInicial(t_EstructuraInicial*);
t_EstructuraInicial* deserializar_EstructuraInicial(char**);

char* serializar_opVarCompartida(t_op_varCompartida* varCompartida);
t_op_varCompartida* deserializar_opVarCompartida(char** package);

char* serializar_opIO(t_op_IO* op_IO);
t_op_IO* deserializar_opIO(char** package);

void enviarERROR(int socket, char* mensajeError);
char* _string_trim(char* texto);
bool _esEspacio(char caracter);

#endif /* STRUCTS_H_ */
