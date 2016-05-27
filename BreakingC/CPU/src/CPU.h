/*
 * CPU.h
 *
 *  Created on: 10/06/2014
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <sockets/Socket.h>
#include <sockets/Socket_Cliente.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include <stdint.h>
#include <signal.h>
#include "comun/Structs.h"
#include "comun/UMV_Structs.h"
#include "Primitivas.h"
#include <stdbool.h>

AnSISOP_funciones functions = {
		.AnSISOP_definirVariable		= definirVariable,
		.AnSISOP_obtenerPosicionVariable= obtenerPosicionVariable,
		.AnSISOP_dereferenciar			= dereferenciar,
		.AnSISOP_asignar				= asignar,
		.AnSISOP_obtenerValorCompartida = obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida = asignarValorCompartida,
		.AnSISOP_irAlLabel 				= irALabel,
		.AnSISOP_llamarSinRetorno		= llamarSinRetorno,
		.AnSISOP_llamarConRetorno 		= llamarConRetorno,
		.AnSISOP_finalizar				= finalizar,
		.AnSISOP_retornar				= retornar,
		.AnSISOP_entradaSalida			= entradaSalida,
		.AnSISOP_imprimir				= imprimir,
		.AnSISOP_imprimirTexto			= imprimirTexto
};

AnSISOP_kernel kernel_functions = {
		.AnSISOP_wait = ansisop_wait,
		.AnSISOP_signal = ansisop_signal
};

int ConectarPCP();
int RecibirPCB(t_pcb** pcb);
t_EstructuraInicial* RecibirQuantum();
int ConectarUMV();
char* ObtenerSentencia(t_pcb* pcb);
bool ObtenerDiccionarioEtiquetas();
void HandshakeUMV();
void CambiarProcesoActivo();
void CambioContexto();

#endif /* CPU_H_ */
