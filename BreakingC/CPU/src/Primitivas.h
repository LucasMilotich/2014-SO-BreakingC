/*
 * Primitivas.h
 *
 *  Created on: 10/06/2014
 *      Author: utnso
 */

#ifndef PRIMITIVAS_H_
#define PRIMITIVAS_H_

#include <parser/parser.h>
#include <parser/sintax.h>
#include <parser/metadata_program.h>
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include <stdio.h>
#include <stdint.h>
#include <comun/UMV_Structs.h>
#include <comun/Structs.h>
#include <sockets/Socket.h>
#include <stdbool.h>
#include <commons/string.h>

t_puntero definirVariable(t_nombre_variable variable);

t_puntero obtenerPosicionVariable(t_nombre_variable variable);
t_valor_variable dereferenciar(t_puntero puntero);
void asignar(t_puntero puntero, t_valor_variable variable);
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
void irALabel(t_nombre_etiqueta etiqueta);
void llamarSinRetorno(t_nombre_etiqueta etiqueta);
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void finalizar();
void retornar(t_valor_variable retorno);
void imprimir(t_valor_variable valor);
void imprimirTexto(char* texto);
void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
void ansisop_wait(t_nombre_semaforo identificador_semaforo);
void ansisop_signal(t_nombre_semaforo identificador_semaforo);

void RecrearDiccionario(bool destruir);

#endif /* PRIMITIVAS_H_ */
