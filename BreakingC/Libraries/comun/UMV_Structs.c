/*
 * UMV_Structs.c
 *
 *  Created on: 09/06/2014
 *      Author: utnso
 */
#include <commons/log.h>
#include "UMV_Structs.h"
#include "../sockets/Socket.h"
#include "Structs.h"

char* serializarSolicitarBytes(t_solicitarBytes* solicitarBytes, uint32_t *operacion) {
	int offset = 0, tmp_size = 0;
	size_t packageSize = sizeof(t_solicitarBytes);
	char * paqueteSerializado = malloc(packageSize + sizeof(uint32_t));

	tmp_size = sizeof(uint32_t);
	memcpy(paqueteSerializado + offset, operacion, tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(solicitarBytes->base);
	memcpy(paqueteSerializado + offset, &(solicitarBytes->base), tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(solicitarBytes->offset);
	memcpy(paqueteSerializado + offset, &(solicitarBytes->offset), tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(solicitarBytes->tamanio);
	memcpy(paqueteSerializado + offset, &(solicitarBytes->tamanio), tmp_size);

	return paqueteSerializado;
}

char* serializarEnviarBytes(t_enviarBytes* enviarBytes, uint32_t *operacion) {
	int offset = 0, tmp_size = 0;
	size_t packageSize = sizeof(t_enviarBytes) + enviarBytes->tamanio;
	char * paqueteSerializado = malloc(packageSize + sizeof(uint32_t));

	tmp_size = sizeof(uint32_t);
	memcpy(paqueteSerializado + offset, operacion, tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(enviarBytes->base);
	memcpy(paqueteSerializado + offset, &(enviarBytes->base), tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(enviarBytes->offset);
	memcpy(paqueteSerializado + offset, &(enviarBytes->offset), tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(enviarBytes->tamanio);
	memcpy(paqueteSerializado + offset, &(enviarBytes->tamanio), tmp_size);
	offset += tmp_size;
	tmp_size = enviarBytes->tamanio;
	memcpy(paqueteSerializado + offset, (enviarBytes->buffer), tmp_size);
	offset++;

	return paqueteSerializado;

}

char* serializarHandshake(t_handshake* handshake, uint32_t *operacion) {
	int offset = 0, tmp_size = 0;
	size_t packageSize = sizeof(handshake->tipo);
	char * paqueteSerializado = malloc(packageSize + sizeof(uint32_t));

	tmp_size = sizeof(uint32_t);
	memcpy(paqueteSerializado + offset, operacion, tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(handshake->tipo);
	memcpy(paqueteSerializado + offset, &(handshake->tipo), tmp_size);

	return paqueteSerializado;
}

char* serializarCambioProcActivo(t_cambio_proc_activo* cambioProcActivo, uint32_t *operacion) {
	int offset = 0, tmp_size = 0;
	size_t packageSize = sizeof(cambioProcActivo->programID)
			+ sizeof(cambioProcActivo->programID);
	char * paqueteSerializado = malloc(packageSize + sizeof(uint32_t));

	tmp_size = sizeof(uint32_t);
	memcpy(paqueteSerializado + offset, operacion, tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(cambioProcActivo->programID);
	memcpy(paqueteSerializado + offset, &(cambioProcActivo->programID),
			tmp_size);

	return paqueteSerializado;
}

char* serializarCrearSegmento(t_crear_segmento* crearSegmento, uint32_t *operacion) {
	int offset = 0, tmp_size = 0;
	size_t packageSize = sizeof(crearSegmento->programID)
			+ sizeof(crearSegmento->tamanio);
	char * paqueteSerializado = malloc(packageSize + sizeof(uint32_t));

	tmp_size = sizeof(uint32_t);
	memcpy(paqueteSerializado + offset, operacion, tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(crearSegmento->programID);
	memcpy(paqueteSerializado + offset, &(crearSegmento->programID), tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(crearSegmento->tamanio);
	memcpy(paqueteSerializado + offset, &(crearSegmento->tamanio), tmp_size);

	return paqueteSerializado;
}

char* serializarDestruirSegmento(t_destruir_segmento* destruirSegmento,	uint32_t *operacion) {
	int offset = 0, tmp_size = 0;
	size_t packageSize = sizeof(destruirSegmento->programID);
	char * paqueteSerializado = malloc(packageSize + sizeof(uint32_t));

	tmp_size = sizeof(uint32_t);
	memcpy(paqueteSerializado + offset, operacion, tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(destruirSegmento->programID);
	memcpy(paqueteSerializado + offset, &(destruirSegmento->programID),
			tmp_size);

	return paqueteSerializado;

}

char* enviarOperacion(uint32_t operacion, void* estructuraDeOperacion, int serverSocket) {
	int packageSize;
	char *paqueteSerializado;
	char* respuestaOperacion;

	switch (operacion) {
	case SOLICITAR_BYTES:
		//info del segundo paquete
		packageSize = sizeof(t_solicitarBytes) + sizeof(uint32_t);
		paqueteSerializado = serializarSolicitarBytes(estructuraDeOperacion, &operacion);

		if ((enviarDatos(serverSocket, &paqueteSerializado, packageSize, NOTHING)) < 0) {
			free(paqueteSerializado);
			return NULL;
		}

		//####		Recibo buffer pedidos		####
		if (recibirDatos(serverSocket, &respuestaOperacion, NULL ) < 0) {
			free(paqueteSerializado);
			return NULL;
		}

		break;
	case ENVIAR_BYTES:
		packageSize = sizeof(t_enviarBytes) + sizeof(uint32_t);
		t_enviarBytes* aux = (t_enviarBytes*)estructuraDeOperacion;
		packageSize += aux->tamanio;
		paqueteSerializado = serializarEnviarBytes(estructuraDeOperacion, &operacion);

		//info del primer paquete
		if ((enviarDatos(serverSocket, &paqueteSerializado, packageSize, NOTHING)) < 0) {
			free(paqueteSerializado);
			return NULL;
		}
		//Recibo la respuesta si fue exitosa
		if ((recibirDatos(serverSocket, &respuestaOperacion, NULL )) < 0) {
			free(paqueteSerializado);
			return NULL;
		}

		break;
	case HANDSHAKE:
		//info del segundo paquete
		packageSize = sizeof(t_handshake) + sizeof(uint32_t);
		paqueteSerializado = serializarHandshake(estructuraDeOperacion,	&operacion);

		//Envio paquete
		if ((enviarDatos(serverSocket, &paqueteSerializado, packageSize, NOTHING)) < 0) {
			free(paqueteSerializado);
			return NULL;
		}

		break;
	case CAMBIO_PROC_ACTIVO:
		//info del segundo paquete
		packageSize = sizeof(t_cambio_proc_activo) + sizeof(uint32_t);
		paqueteSerializado = serializarCambioProcActivo(estructuraDeOperacion, &operacion);

		//Envio paquete
		if ((enviarDatos(serverSocket, &paqueteSerializado, packageSize, NOTHING)) < 0) {
			free(paqueteSerializado);
			return NULL;
		}

		break;
	case CREAR_SEGMENTO:
		//info del segundo paquete
		packageSize = sizeof(t_crear_segmento) + sizeof(uint32_t);
		paqueteSerializado = serializarCrearSegmento(estructuraDeOperacion,	&operacion);

		//Envio paquete
		if ((enviarDatos(serverSocket, &paqueteSerializado, packageSize, NOTHING)) < 0) {
			free(paqueteSerializado);
			return NULL;
		}
		//Recibo el valor respuesta de la operación
		respuestaOperacion = malloc(sizeof(int));
		if (recibirDatos(serverSocket, &respuestaOperacion, NULL ) < 0) {
			respuestaOperacion[0] = -1;
			respuestaOperacion[1] = '\0';
			free(paqueteSerializado);
			return respuestaOperacion;
		}

		break;
	case DESTRUIR_SEGMENTO:
		//info del segundo paquete
		packageSize = sizeof(t_destruir_segmento) + sizeof(uint32_t);
		paqueteSerializado = serializarDestruirSegmento(estructuraDeOperacion, &operacion);

		//envio paquete
		if ((enviarDatos(serverSocket, &paqueteSerializado, packageSize, NOTHING)) < 0) {
			free(paqueteSerializado);
			return NULL;
		}

		//Recibo respuesta
		if (recibirDatos(serverSocket, &respuestaOperacion, NULL ) < 0) {
			free(paqueteSerializado);
			return NULL;
		}

		break;
	default:
		printf("Operacion no admitida");
		break;
	}

	free(paqueteSerializado);
	return respuestaOperacion;
}
