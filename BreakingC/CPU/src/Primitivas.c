/*
 * Primitivas.c
 *
 *  Created on: 10/06/2014
 *      Author: utnso
 */

#include "Primitivas.h"

#define TAMANIO_VARIABLE 5
extern t_dictionary* dic_variables;
extern char* etiquetas;
extern t_pcb* pcb;
extern int socket_PCP;
extern int socket_UMV;
extern t_log* Log;
extern char operacion;

t_puntero definirVariable(t_nombre_variable variable) {
	t_puntero offset = 0;
	char* buffer = malloc(TAMANIO_VARIABLE);
	log_debug(Log, "Se define la variable %c", variable);
	buffer[0] = variable;
	offset = (pcb->cursor_stack - pcb->seg_stack) + pcb->tamanio_contexto * TAMANIO_VARIABLE;
	t_enviarBytes* enviarBytes = malloc(sizeof(t_enviarBytes));
	enviarBytes->base = pcb->seg_stack;
	enviarBytes->offset = offset;
	enviarBytes->tamanio = TAMANIO_VARIABLE;
	enviarBytes->buffer = buffer;
	char *resp = enviarOperacion(ENVIAR_BYTES, enviarBytes, socket_UMV);
	if (resp[0] == -1){
		operacion = ERROR;
		free(resp);
		free(buffer);
		free(enviarBytes);
		return -1;
	}

	buffer[1] = '\0';
	t_variable *est_variable = variable_create(buffer, offset);
	dictionary_put(dic_variables, est_variable->nombre, est_variable);
	log_debug(Log,
			"Variable '%c' definida en la posicion %d con respecto al segmento de stack",
			variable, offset);
	pcb->tamanio_contexto++;
	free(resp);
	free(buffer);
	free(enviarBytes);
	return offset;
}

t_puntero obtenerPosicionVariable(t_nombre_variable variable) {
	t_variable* est_variable;
	char* nom = malloc(2);
	nom[0] = variable;
	nom[1] = '\0';
	log_debug(Log, "Se obtiene la posicion de '%c'", variable);
	est_variable = (t_variable*) dictionary_get(dic_variables, nom);
	log_debug(Log, "La posicion de '%c' es %u", variable, est_variable->offset);
	free(nom);
	return est_variable->offset;
}

t_valor_variable dereferenciar(t_puntero puntero) {
	char* buffer;
	t_valor_variable valor = 0;
	log_debug(Log, "Dereferenciando %d", puntero);
	t_solicitarBytes* solicitar = malloc(sizeof(solicitar));
	solicitar->base = pcb->seg_stack;
	solicitar->offset = puntero;
	solicitar->tamanio = 5;
	buffer = enviarOperacion(SOLICITAR_BYTES, solicitar, socket_UMV);
	if (buffer[0] == -1){
		operacion = ERROR;
		free(buffer);
		free(solicitar);
		return -1;
	}
	memcpy(&valor, buffer + 1, sizeof(t_valor_variable));
	log_debug(Log, "El valor es %d", valor);
	free(buffer);
	free(solicitar);
	return valor;
}

void asignar(t_puntero puntero, t_valor_variable variable) {
	log_debug(Log, "Asignando en %d el valor %d", puntero, variable);
	t_enviarBytes* enviar = malloc(sizeof(t_enviarBytes));
	enviar->base = pcb->seg_stack;
	enviar->offset = puntero + 1;
	enviar->tamanio = 4;
	enviar->buffer = malloc(sizeof(t_valor_variable));
	memcpy(enviar->buffer, &variable, sizeof(t_valor_variable));
	char *resp = enviarOperacion(ENVIAR_BYTES, enviar, socket_UMV);
	if (resp[0] == -1){
		operacion = ERROR;
		free(resp);
		free(enviar->buffer);
		free(enviar);
		return;
	}
	log_debug(Log, "Valor asignado");
	free(enviar->buffer);
	free(enviar);
	return;
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable) {
	char* buffer;
	t_valor_variable valor = 0;
	log_debug(Log, "Obteniendo el valor de la variable compartida '%s'",
			variable);
	if (enviarDatos(socket_PCP, &variable, strlen(variable) + 1,
			LEER_VAR_COMPARTIDA) < 0)
		return -1;
	if (recibirDatos(socket_PCP, &buffer, NULL ) < 0)
		return -1;

	memcpy(&valor, buffer, sizeof(t_valor_variable));
	log_debug(Log, "El valor de '%s' es %d", variable, valor);
	free(buffer);
	return valor;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable,
		t_valor_variable valor) {
	int longitud = strlen(variable) + 1 + 8; //longitud del nombre de la variable + \0 + 8B de los int de la estructura
	char* buffer;
	log_debug(Log, "Asignando el valor %d a la variable compartida '%s'", valor,
			variable);
	t_op_varCompartida* varCompartida = malloc(sizeof(t_op_varCompartida));
	varCompartida->longNombre = strlen(variable) + 1;
	varCompartida->nombre = malloc(strlen(variable) + 1);
	strcpy(varCompartida->nombre, variable);
	varCompartida->valor = valor;
	buffer = serializar_opVarCompartida(varCompartida);

	if (enviarDatos(socket_PCP, &buffer, longitud, ASIG_VAR_COMPARTIDA) < 0)
		return -1;
	free(buffer);
	free(varCompartida->nombre);
	free(varCompartida);
	log_debug(Log, "Valor asignado");
	//RECIBIR SUCCESS
//	if (recibirDatos(socket_PCP, &buffer, NULL) < 0)
//		return -1;
	return valor;
}

void irALabel(t_nombre_etiqueta etiqueta) {
	pcb->PC = metadata_buscar_etiqueta(etiqueta, etiquetas, pcb->tamanio_index_etiq);
	log_debug(Log, "El valor del PC ahora es %d", pcb->PC);
	return;
}

void llamarSinRetorno(t_nombre_etiqueta etiqueta) {
	u_int32_t offset = 0;
	log_debug(Log,
			"Reservando espacio y cambiando al nuevo contexto de ejecucion");
	offset = (pcb->cursor_stack - pcb->seg_stack) + TAMANIO_VARIABLE * pcb->tamanio_contexto;
	t_enviarBytes* enviar = malloc(sizeof(t_enviarBytes));
	enviar->base = pcb->seg_stack;
	enviar->offset = offset;
	enviar->tamanio = 8;
	enviar->buffer = malloc(8);
	memcpy(enviar->buffer, &(pcb)->cursor_stack, 4);
	memcpy(enviar->buffer + 4, &(pcb)->PC, 4);
	char* resp = enviarOperacion(ENVIAR_BYTES, enviar, socket_UMV);
	if (resp[0] == -1){
		operacion = ERROR;
		free(resp);
		free(enviar->buffer);
		free(enviar);
		return;
	}
	pcb->cursor_stack += offset + 8;
	pcb->tamanio_contexto = 0;
	RecrearDiccionario(true);
	irALabel(etiqueta);
	free(enviar->buffer);
	free(enviar);
	return;
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar) {
	u_int32_t offset = 0;
	log_debug(Log,
			"Reservando espacio y cambiando al nuevo contexto de ejecucion");
	offset = (pcb->cursor_stack - pcb->seg_stack) + TAMANIO_VARIABLE * pcb->tamanio_contexto;
	t_enviarBytes* enviar = malloc(sizeof(t_enviarBytes));
	enviar->base = pcb->seg_stack;
	enviar->offset = offset;
	enviar->tamanio = 12;
	enviar->buffer = malloc(12);
	memcpy(enviar->buffer, &(pcb)->cursor_stack, 4);
	memcpy(enviar->buffer + 4, &(pcb)->PC, 4);
	memcpy(enviar->buffer + 8, &donde_retornar, 4); //tal vez haya que sumar 1 a donde_retornar por el byte del nombre de variable
	char* resp = enviarOperacion(ENVIAR_BYTES, enviar, socket_UMV);
	if (resp[0] == -1){
		operacion = ERROR;
		free(resp);
		free(enviar->buffer);
		free(enviar);
		return;
	}
	pcb->cursor_stack += offset + 12;
	pcb->tamanio_contexto = 0;
	RecrearDiccionario(true);
	irALabel(etiqueta);
	free(enviar->buffer);
	free(enviar);
	return;
}

void finalizar() {
	if (pcb->cursor_stack == pcb->seg_stack) {
		log_debug(Log, "Se finaliza el programa");
		operacion = EXIT; //FINALIZAR EJECUCION
		return;
	}
	log_debug(Log, "Retornando al contexto de ejecucion anterior");
	char* buffer;
	u_int32_t cursorActual = pcb->cursor_stack;
	t_solicitarBytes* solicitar = malloc(sizeof(t_solicitarBytes));
	solicitar->base = pcb->seg_stack;
	solicitar->offset = pcb->cursor_stack - pcb->seg_stack - 8;
	solicitar->tamanio = 8;
	buffer = enviarOperacion(SOLICITAR_BYTES, solicitar, socket_UMV);
	if (buffer[0] == -1){
		operacion = ERROR;
		free(buffer);
		free(solicitar);
		return;
	}
	memcpy(&(pcb)->cursor_stack, buffer, 4);
	memcpy(&(pcb)->PC, buffer + 4, 4);
	pcb->tamanio_contexto = ((cursorActual - 8) - pcb->cursor_stack) / TAMANIO_VARIABLE;
	RecrearDiccionario(true);
	free(solicitar);
	free(buffer);
	return;
}

void retornar(t_valor_variable retorno) {
	char* buffer;
	t_puntero pos;
	u_int32_t cursorActual = pcb->cursor_stack;
	log_debug(Log, "Retornando al contexto de ejecucion anterior");
	t_solicitarBytes* solicitar = malloc(sizeof(t_solicitarBytes));
	solicitar->base = pcb->seg_stack;
	solicitar->offset = pcb->cursor_stack - pcb->seg_stack - 12;
	solicitar->tamanio = 12;
	buffer = enviarOperacion(SOLICITAR_BYTES, solicitar, socket_UMV);
	if (buffer[0] == -1){
		operacion = ERROR;
		free(buffer);
		free(solicitar);
		return;
	}
	memcpy(&(pcb)->cursor_stack, buffer, 4);
	memcpy(&(pcb)->PC, buffer + 4, 4);
	memcpy(&pos, buffer + 8, 4);
	asignar(pos, retorno);
	pcb->tamanio_contexto = ((cursorActual - 12) - pcb->cursor_stack) / TAMANIO_VARIABLE;
	RecrearDiccionario(true);
	log_debug(Log, "El valor retornado es %d", retorno);
	free(solicitar);
	free(buffer);
	return;
}

void imprimir(t_valor_variable valor) {
	log_debug(Log,
			"Enviando al kernel el valor %d que se mostrara por pantalla",
			valor);
	char* buffer = malloc(sizeof(t_valor_variable));
	memcpy(buffer, &valor, sizeof(t_valor_variable));
	enviarDatos(socket_PCP, &buffer, sizeof(t_valor_variable), IMPRIMIR_VALOR);
	log_debug(Log, "Valor enviado");
	free(buffer);
	return;
}

void imprimirTexto(char* texto) {
	log_debug(Log,
			"Enviando al kernel una cadena de texto que se mostrara por pantalla");
	texto = _string_trim(texto);
	log_trace(Log, "La cadena es:\n%s", texto);
	enviarDatos(socket_PCP, &texto, strlen(texto) + 1, IMPRIMIR_TEXTO);
	log_debug(Log, "Cadena enviada");
	return;
}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo) {
	char* buffer;
	operacion = IO;
	log_debug(Log, "Se efectua la operacion de Entrada/Salida");
	t_op_IO* op_IO = malloc(sizeof(t_op_IO));
	op_IO->dispositivo = malloc(strlen(dispositivo) + 1);
	strcpy(op_IO->dispositivo, dispositivo);
	op_IO->tiempo = tiempo;
	buffer = serializar_opIO(op_IO);
	enviarDatos(socket_PCP, &buffer, 8 + strlen(dispositivo) + 1, operacion);
	log_debug(Log, "Dispositivo '%s' con un tiempo de %d enviados al kernel",
			dispositivo, tiempo);
	free(buffer);
	free(op_IO->dispositivo);
	free(op_IO);
	return;
}

void ansisop_wait(t_nombre_semaforo identificador_semaforo) {
	char* buffer;
	char op;
	log_debug(Log, "Enviado al kernel funcion WAIT para el semaforo '%s'", identificador_semaforo);
	enviarDatos(socket_PCP, &identificador_semaforo, strlen(identificador_semaforo) + 1, WAIT);
	log_debug(Log, "Esperando respuesta del kernel");
	recibirDatos(socket_PCP, &buffer, &op);

	if (op != NOTHING)
		operacion = op;

	if (op == WAIT)
		log_debug(Log, "El proceso queda bloqueado hasta que se haga un SIGNAL a '%s'", identificador_semaforo);

	free(buffer);
	return;
}

void ansisop_signal(t_nombre_semaforo identificador_semaforo) {
	log_debug(Log, "Enviado al kernel funcion SIGNAL para el semaforo '%s'", identificador_semaforo);
	enviarDatos(socket_PCP, &identificador_semaforo, strlen(identificador_semaforo) + 1, SIGNAL);
	return;
}

/*
 * FUNCION QUE REGENERA EL DICCIONARIO DE VARIABLES
 * NO ES UNA PRIMITIVA
 */

void RecrearDiccionario(bool destruir) {
	uint32_t pos = 0;
	t_variable* variable;
	char* buffer;
	int i, tamanio = 0;
	if (destruir)
		dictionary_destroy_and_destroy_elements(dic_variables, (void*) variable_destroy);
	log_debug(Log, "Recreando diccionario de variables");
	dic_variables = dictionary_create();
	tamanio = pcb->tamanio_contexto * TAMANIO_VARIABLE;
	if (tamanio == 0) {
		log_info(Log, "Se ha creado el diccionario de variables vacio");
		return;
	}
	char* var = malloc(2);
	t_solicitarBytes* solicitarBytes = malloc(sizeof(t_solicitarBytes));
	solicitarBytes->base = pcb->seg_stack;
	solicitarBytes->offset = pcb->cursor_stack - pcb->seg_stack;
	solicitarBytes->tamanio = tamanio;
	buffer = enviarOperacion(SOLICITAR_BYTES, solicitarBytes, socket_UMV);
	if (buffer[0] == -1){
		operacion = ERROR;
		free(buffer);
		free(solicitarBytes);
		free(var);
		return;
	}

	for (i = 0; i < pcb->tamanio_contexto; i++) {
		var[0] = *(buffer + i * TAMANIO_VARIABLE);
		var[1] = '\0';
		pos = (pcb->cursor_stack - pcb->seg_stack) + i * TAMANIO_VARIABLE;
		variable = variable_create(var, pos);
		dictionary_put(dic_variables, variable->nombre, variable);
	}
	log_debug(Log, "Creado con exito");
	free(buffer);
	free(solicitarBytes);
	free(var);
	return;
}
