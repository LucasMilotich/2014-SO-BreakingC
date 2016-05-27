/*
 * Structs.c
 *
 *  Created on: 01/06/2014
 *      Author: fbustos
 */

#include "Structs.h"

t_pcb* deserializar_pcb(char** package) {
	t_pcb *pcb = malloc(sizeof(t_pcb));
	int offset = 0;
	int tmp_size = sizeof(uint32_t);

	memcpy(&pcb->pcb_id, *package + offset, tmp_size);
	offset += tmp_size;
	memcpy(&pcb->seg_codigo, *package + offset, tmp_size);
	offset += tmp_size;
	memcpy(&pcb->seg_stack, *package + offset, tmp_size);
	offset += tmp_size;
	memcpy(&pcb->cursor_stack, *package + offset, tmp_size);
	offset += tmp_size;
	memcpy(&pcb->index_codigo, *package + offset, tmp_size);
	offset += tmp_size;
	memcpy(&pcb->index_etiq, *package + offset, tmp_size);
	offset += tmp_size;
	memcpy(&pcb->PC, *package + offset, tmp_size);
	offset += tmp_size;
	memcpy(&pcb->tamanio_contexto, *package + offset, tmp_size);
	offset += tmp_size;
	memcpy(&pcb->tamanio_index_etiq, *package + offset, tmp_size);
	offset += tmp_size;
	memcpy(&pcb->peso, *package + offset, tmp_size);
	return pcb;
}

char* serializar_pcb(t_pcb* pcb) {
	int offset = 0, tmp_size = 0;
	char * paqueteSerializado = malloc(sizeof(t_pcb));

	tmp_size = sizeof(pcb->pcb_id);
	memcpy(paqueteSerializado + offset, &(pcb->pcb_id), tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(pcb->seg_codigo);
	memcpy(paqueteSerializado + offset, &(pcb->seg_codigo), tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(pcb->seg_stack);
	memcpy(paqueteSerializado + offset, &(pcb->seg_stack), tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(pcb->cursor_stack);
	memcpy(paqueteSerializado + offset, &(pcb->cursor_stack), tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(pcb->index_codigo);
	memcpy(paqueteSerializado + offset, &(pcb->index_codigo), tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(pcb->index_etiq);
	memcpy(paqueteSerializado + offset, &(pcb->index_etiq), tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(pcb->PC);
	memcpy(paqueteSerializado + offset, &(pcb->PC), tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(pcb->tamanio_contexto);
	memcpy(paqueteSerializado + offset, &(pcb->tamanio_contexto), tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(pcb->tamanio_index_etiq);
	memcpy(paqueteSerializado + offset, &(pcb->tamanio_index_etiq), tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(pcb->peso);
	memcpy(paqueteSerializado + offset, &(pcb->peso), tmp_size);

	return paqueteSerializado;
}

char* serializar_EstructuraInicial(t_EstructuraInicial * estructuraInicial) {
	int offset = 0, tmp_size = 0;
	char * paqueteSerializado = malloc(sizeof(t_EstructuraInicial));

	tmp_size = sizeof(estructuraInicial->Quantum);
	memcpy(paqueteSerializado + offset, &(estructuraInicial->Quantum),
			tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(estructuraInicial->RetardoQuantum);
	memcpy(paqueteSerializado + offset, &(estructuraInicial->RetardoQuantum),
			tmp_size);

	return paqueteSerializado;
}

t_EstructuraInicial* deserializar_EstructuraInicial(char** package) {
	t_EstructuraInicial * estructuraInicial = malloc(
			sizeof(t_EstructuraInicial));
	int offset = 0;
	int tmp_size = sizeof(uint32_t);

	memcpy(&estructuraInicial->Quantum, *package + offset, tmp_size);
	offset += tmp_size;
	memcpy(&estructuraInicial->RetardoQuantum, *package + offset, tmp_size);

	return estructuraInicial;
}

char* serializar_opVarCompartida(t_op_varCompartida* varCompartida) {
	int offset = 0, tmp_size = 0;
	char * paqueteSerializado = malloc(8 + (varCompartida->longNombre));

	tmp_size = sizeof(varCompartida->longNombre);
	memcpy(paqueteSerializado + offset, &(varCompartida->longNombre), tmp_size);
	offset += tmp_size;
	tmp_size = varCompartida->longNombre;
	memcpy(paqueteSerializado + offset, varCompartida->nombre, tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(varCompartida->valor);
	memcpy(paqueteSerializado + offset, &(varCompartida->valor), tmp_size);

	return paqueteSerializado;
}

t_op_varCompartida* deserializar_opVarCompartida(char** package) {
	t_op_varCompartida* varCompartida = malloc(sizeof(t_op_varCompartida));
	int offset = 0;
	int tmp_size = sizeof(uint32_t);

	memcpy(&varCompartida->longNombre, *package + offset, tmp_size);
	offset += tmp_size;
	tmp_size = varCompartida->longNombre;
	varCompartida->nombre = malloc(tmp_size);
	memcpy(varCompartida->nombre, *package + offset, tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(uint32_t);
	memcpy(&varCompartida->valor, *package + offset, tmp_size);

	return varCompartida;
}

char* serializar_opIO(t_op_IO* op_IO){
	int longitud = strlen(op_IO->dispositivo) + 1;
	int offset = 0, tmp_size = 0;
	char * paqueteSerializado = malloc(8 + longitud);

	tmp_size = 4;
	memcpy(paqueteSerializado + offset, &(longitud), tmp_size);
	offset += tmp_size;
	tmp_size = longitud;
	memcpy(paqueteSerializado + offset, op_IO->dispositivo, tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(op_IO->tiempo);
	memcpy(paqueteSerializado + offset, &(op_IO->tiempo), tmp_size);

	return paqueteSerializado;
}

t_op_IO* deserializar_opIO(char** package){
	t_op_IO* op_IO = malloc(sizeof(t_op_IO));
	int longitud, offset = 0;
	int tmp_size = sizeof(uint32_t);

	memcpy(&longitud, *package + offset, tmp_size);
	offset += tmp_size;
	tmp_size = longitud;
	op_IO->dispositivo = malloc(tmp_size);
	memcpy(op_IO->dispositivo, *package + offset, tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(uint32_t);
	memcpy(&op_IO->tiempo, *package + offset, tmp_size);

	return op_IO;
}

t_variable *variable_create(char *nombre, uint32_t offset){
	t_variable *new = malloc( sizeof(t_variable) );
	new->nombre = strdup(nombre);
	new->offset = offset;
	return new;
}

void variable_destroy(t_variable *self){
	free(self->nombre);
	free(self);
}

void enviarERROR(int socket, char* mensajeError){
	enviarDatos(socket, &mensajeError, strlen(mensajeError) + 1, ERROR);
}

char* _string_trim(char* texto){
    int i;
    while (_esEspacio (*texto)) texto++;   //Anda a el primer no-espacio
    for (i = strlen (texto) - 1; i>0 && (_esEspacio (texto[i])); i--);   //y de atras para adelante
    texto[i + 1] = '\0';
    return texto;
}

bool _esEspacio(char caracter){
	return caracter==' ' || caracter=='\t' || caracter=='\f' || caracter=='\r' || caracter=='\v';
}
