/*
 * funcionesDeUMV.h
 *
 *  Created on: 27/06/2014
 *      Author: utnso
 */

#ifndef FUNCIONESDEUMV_H_
#define FUNCIONESDEUMV_H_
#include "estructuras.h"

char* solicitarBytes(t_solicitarBytes *solicitarBytes, int* procesoActivo);
char* enviarBytes(t_enviarBytes *enviarBytes, int procesoActivo);
int crearSegmento(t_crear_segmento* segmentoACrear);
int destruirSegmento(t_destruir_segmento* segmentoADestruir);
int cambiarProcesoActivo(t_cambio_proc_activo* cambioProcesoActivo,
		int* procesoActivo);
int handshake(t_handshake* handshake, int* tipoProceso);
void compactar();
int  compactacion();
void dump(int,int);
#endif /* FUNCIONESDEUMV_H_ */
