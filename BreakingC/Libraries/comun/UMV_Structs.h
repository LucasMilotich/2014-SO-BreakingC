/*
 * UMV_Structs.h
 *
 *  Created on: 09/06/2014
 *      Author: utnso
 */

#ifndef UMV_STRUCTS_H_
#define UMV_STRUCTS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>


enum OPERACIONES  {
			SOLICITAR_BYTES=1,
			ENVIAR_BYTES,
			HANDSHAKE,
			CAMBIO_PROC_ACTIVO,
			CREAR_SEGMENTO,
			DESTRUIR_SEGMENTO
        };
enum TIPO_HANDSHAKE  {
			KERNEL=0,
			CPU
        };

typedef struct _package_solicitarBytes {
	uint32_t base;
	uint32_t offset;
	uint32_t tamanio;
} t_solicitarBytes;

typedef struct _package_enviarBytes {
	uint32_t base;
	uint32_t offset;
	uint32_t tamanio;
	char* buffer;
} t_enviarBytes;

typedef struct _package_handshake {
	//uint32_t programID;
	uint32_t tipo;
} t_handshake;

typedef struct _package_cambio_proc_activo {
	uint32_t programID;
} t_cambio_proc_activo;

typedef struct _package_crear_segmento {
	uint32_t programID;
	uint32_t tamanio;
} t_crear_segmento;

typedef struct _package_destruir_segmento {
	uint32_t programID;
} t_destruir_segmento;

char* serializarSolicitarBytes(t_solicitarBytes*,uint32_t *);
char* serializarEnviarBytes(t_enviarBytes*,uint32_t *);
char* serializarHandshake(t_handshake*,uint32_t *);
char* serializarCambioProcActivo(t_cambio_proc_activo*,uint32_t *);
char* serializarCrearSegmento(t_crear_segmento*,uint32_t *);
char* serializarDestruirSegmento(t_destruir_segmento*,uint32_t *);

char* enviarOperacion(uint32_t, void*, int);

#endif /* UMV_STRUCTS_H_ */
