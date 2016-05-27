/*
 * servidor.c
 *
 *  Created on: 29/04/2014
 *      Author: utnso
 */

#include "estructuras.h"
#include "servidor.h"
#include "../funcionesDeUMV.h"
#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <commons/config.h>
#include <sockets/Socket.h>
#include <sockets/Socket_Servidor.h>

int deserializarYejecutar(char**, int, int*, int*);
extern t_list * UMV;
extern t_log* ptrLog;

extern t_config* config;

pthread_t UMV_Servidor() {
	/*pthread_t hilo;
	 char* PUERTO = config_get_string_value(config, "PUERTO");
	 char* IP = config_get_string_value(config, "IP");
	 struct addrinfo hints;
	 struct addrinfo *serverInfo;
	 memset(&hints, 0, sizeof(hints));
	 hints.ai_family = AF_INETAF_UNSPEC;		//PROBAR AF_INET
	 //	hints.ai_flags = SOCK_STREAM;// Asigna el address del localhost: 127.0.0.1
	 hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	 getaddrinfo(IP, PUERTO, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE

	 // Creo socket para escuchar

	 int listenningSocket;
	 listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
	 serverInfo->ai_protocol);
	 log_info(ptrLog, "Se creo el socket");
	 // Bindeo el ListenningSocket con la serverInfo

	 bind(listenningSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	 freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar
	 log_info(ptrLog, "Bindeo el socket");
	 //Lo pongo a escuchar
	 listen(listenningSocket, BACKLOG); // IMPORTANTE: listen() es una syscall BLOQUEANTE.

	 log_info(ptrLog, "Pongo a escuchar el socket");
	 log_debug(ptrLog, "Esperando conexiones...");
	 // Creo un socket extra para aceptar la conexion.listenningSocket se encarga solo de escuchar.

	 struct sockaddr_in addr; // Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	 socklen_t addrlen = sizeof(addr);
	 int socketCliente;
	 int *new_sock;
	 log_info(ptrLog, "Creo el socket cliente");
	 ///////////////////////////////////////////////////////
	 while ((socketCliente = accept(listenningSocket, (struct sockaddr *) &addr,
	 &addrlen))) {
	 log_debug(ptrLog, "Conexion establecida");
	 log_debug(ptrLog, "El id del socket: %d", socketCliente);
	 //Creo un hilo por conexion
	 pthread_t sniffer_thread;
	 agregado memory leak
	 new_sock = malloc(sizeof(int));
	 ----------------
	 *new_sock = socketCliente;

	 if ((pthread_create(&sniffer_thread, NULL, connection_handler,
	 (void*) new_sock)) != 0) {
	 perror("could not create thread");
	 log_error(ptrLog, "No se pudo crear el hilo");
	 return 1;
	 }
	 log_info(ptrLog, "Thread creado.\n");
	 hilo = sniffer_thread;

	 agregado memory leak
	 *
	 * VERIFICAR SI FALTA FREE DE new_sock
	 *
	 *
	 *

	 }
	 close(socketCliente);
	 close(listenningSocket);
	 log_info(ptrLog, "Cierro sockets");
	 Agregado memory leak
	 free(PUERTO);
	 free(IP);
	 --------------------
	 return hilo;*/

	pthread_t hilo;
	int PUERTO = config_get_int_value(config, "PUERTO");
	int socketEscucha, socketNuevaConexion;

	if ((socketEscucha = Abrir_Conexion_Servidor(PUERTO)) < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	log_debug(ptrLog, "Esperando conexiones");

	int *new_sock;

	///////////////////////////////////////////////////////
	while ((socketNuevaConexion = Aceptar_Conexion_Cliente(socketEscucha))) {
		log_info(ptrLog, "Conexion establecida");
		log_debug(ptrLog, "El id del socket: %d", socketNuevaConexion);
		//Creo un hilo por conexion
		pthread_t sniffer_thread;
		/*agregado memory leak*/
		new_sock = malloc(sizeof(int));
		/*----------------*/
		*new_sock = socketNuevaConexion;

		if ((pthread_create(&sniffer_thread, NULL, connection_handler,
				(void*) new_sock)) != 0) {
			log_error(ptrLog,"No se pudo crear el hilo");

		}
		printf("Thread creado. Una nueva CPU o un kernel se conecto. \n");
		hilo = sniffer_thread;

		/*agregado memory leak
		 *
		 * VERIFICAR SI FALTA FREE DE new_sock
		 *
		 *
		 * */

	}
	close(socketNuevaConexion);
	close(socketEscucha);
	return hilo;
}

void *connection_handler(void *socketEscucha) {
	//Get the socket descriptor
	/*HANDSHAKE 0-Kernel 1-CPU*/
	int tipoProceso = -1;
	/*Proceso activo*/
	int procesoActivo = -1;
	int sigoEscuchando = 1;
	int sock = *(int*) socketEscucha;

	/*mememory leak
	 *
	 * VERIFICAR
	 * podria hacer el free del socketEscucha en vez del new_sock mas arriba
	 *
	 * */
	log_info(ptrLog, "Cliente Conectado.\n");

	char *package;

	log_info(ptrLog, "Esperando valores...\n");

	while (sigoEscuchando > -1) {
		//####		Recibo el primer paquete		####
	//	char* paqueteLongitud = malloc(sizeof(int));

		if (recibirDatos(sock, &package, NULL ) > 0) {

			log_info(ptrLog, "Recibi el paquete de datos.");
			//log_info(ptrLog, "Paso a deserializar");
			sigoEscuchando = deserializarYejecutar(&package, sock, &tipoProceso,
					&procesoActivo);

			//Libero memoria.

			free(package);

			//free(paqueteLongitud);

		} else {
			log_error(ptrLog,
					"Error al recibir socket o cliente desconectado.");
			sigoEscuchando = -1;
			//free(paqueteLongitud);
		}
	}
	close(sock);
	log_debug(ptrLog, "Cliente desconectado");
	return NULL;
}

int deserializarYejecutar(char** package, int sock, int* tipoProceso,
		int* procesoActivo) {
	//int *operacion;
	int operacion=0;
	int offset = 0;
	int respuestaINT;
	char* algo = malloc (sizeof(uint32_t));
	int i;
	log_info(ptrLog, "Traspaos de buffer a operacion");
	/*algo[0] = *package[0];
	 operacion = algo[0];*/
	char * bufferRespuesta;
	memcpy(algo, (*package + offset), sizeof(uint32_t));
	//log_debug(ptrLog, "Ejecutamos la operacion:%d", operacion);
	memcpy(&operacion,algo,sizeof(uint32_t));
	//operacion = algo[0];
	free(algo);
	char* respuestaOperacion = malloc(sizeof(int));
	/*memory leak
	 *
	 * VERIFICAR
	 *
	 * deberia hacer un free de respuesOperacion cada vez que entra en cada case
	 *
	 *
	 * */
	//Compruebo que me haya hecho el handshake
	//if (*tipoProceso != KERNEL || *tipoProceso != CPU) {
	//le asigno -1 para que cuando vaya al case, caiga en el default
	//return  -1;
	//}
	log_info(ptrLog, "Antes de decidir la operacion");
	switch (operacion) {
	case 1:
		log_info(ptrLog, "En el case (SERVIDOR)");
		log_debug(ptrLog, "Solicitar Bytes");
		log_debug(ptrLog, "Operacion : %u ", *(*package));
		log_debug(ptrLog, "Base : %u ", *(*package + sizeof(uint32_t)));
		log_debug(ptrLog, "Offset : %u ",
				*(*package + sizeof(uint32_t) + sizeof(uint32_t)));
		log_debug(ptrLog, "Tamanio: %u ",
				*(*package + sizeof(uint32_t) + sizeof(uint32_t)
						+ sizeof(uint32_t)));

		t_solicitarBytes *estructuraSolicitarBytes = deserializarSolicitarBytes(
				package);

		log_debug(ptrLog,
				"Antes de ejecutar la fn solicitarBytes - Imprimiendo la estructura");
		log_debug(ptrLog, "Base : %u ", estructuraSolicitarBytes->base);
		log_debug(ptrLog, "Offset : %u ", estructuraSolicitarBytes->offset);
		log_debug(ptrLog, "Tamanio : %u ", estructuraSolicitarBytes->tamanio);

		if (*tipoProceso == KERNEL || *tipoProceso == CPU) {
			//bufferRespuesta=malloc(estructuraSolicitarBytes->tamanio);
			bufferRespuesta = solicitarBytes(estructuraSolicitarBytes,
					procesoActivo);
			if (bufferRespuesta[0] == -1) {
				log_error(ptrLog, "SEGFAULT");
				enviarDatos(sock, &bufferRespuesta,
						estructuraSolicitarBytes->tamanio, 0);
				log_info(ptrLog,"Envie el mensaje de error");
			} else {
				enviarDatos(sock, &bufferRespuesta,
						estructuraSolicitarBytes->tamanio, 0);
				log_debug(ptrLog, "Lo que se obtuvo es: ");
				for (i = 0; i < estructuraSolicitarBytes->tamanio; ++i)
					log_debug(ptrLog, "%x ", bufferRespuesta[i]);
				log_info(ptrLog, "SolicitarBytes en server correcto");
			}
			free(bufferRespuesta);
			free(estructuraSolicitarBytes);
			return 1;
		}

		else {
			log_error(ptrLog,
					"Se quiere ejecutar una operacion donde no hay una CPU o un Kernel");
			log_error(ptrLog, "Se envia la respuesta fallida");
			bufferRespuesta = malloc(4);
			respuestaINT = -1;
			memcpy(bufferRespuesta, &respuestaINT, sizeof(int));
			enviarDatos(sock, &bufferRespuesta, sizeof(int), 0);
			free(estructuraSolicitarBytes);
			free(bufferRespuesta);
			return 1;	//VERIFICAR
		}

	case 2:
		log_debug(ptrLog, "En el case (SERVIDOR)");
		log_debug(ptrLog, "Enviar Bytes");
		log_debug(ptrLog, "Operacion: %u", *(*package));
		log_debug(ptrLog, "Base: %u", *(*package + sizeof(uint32_t)));
		log_debug(ptrLog, "Offset: %u",
				*(*package + sizeof(uint32_t) + sizeof(uint32_t)));
		log_debug(ptrLog, "Tamanio: %u",
				*(*package + sizeof(uint32_t) + sizeof(uint32_t)
						+ sizeof(uint32_t)));
		log_debug(ptrLog, "%u",
				*(*package + sizeof(uint32_t) + sizeof(uint32_t)
						+ sizeof(uint32_t) + sizeof(uint32_t)));

		t_enviarBytes *estructuraEnviarBytes = deserializarEnviarBytes(package);
		log_debug(ptrLog,
				"Antes de ejecutar la fn enviarBytes - Imprimiendo la estructura");
		log_debug(ptrLog, "Base: %u ", estructuraEnviarBytes->base);
		log_debug(ptrLog, "Offset: %u ", estructuraEnviarBytes->offset);
		log_debug(ptrLog, "Tamanio: %u ", estructuraEnviarBytes->tamanio);
		log_debug(ptrLog, "Se recibio en el buffer lo siguiente:");
		for (i = 0; i < estructuraEnviarBytes->tamanio; ++i)
			log_debug(ptrLog, "%x ", estructuraEnviarBytes->buffer[i]);


		if (*tipoProceso == KERNEL || *tipoProceso == CPU) {
			log_info(ptrLog, "Se procede a enviar bytes");
			bufferRespuesta = enviarBytes(estructuraEnviarBytes,
					*procesoActivo);
			if (bufferRespuesta[0] == -1) {
				log_error(ptrLog, "SEGFAULT");
				enviarDatos(sock, &bufferRespuesta, sizeof(int), 0);
			} else {

				enviarDatos(sock, &bufferRespuesta, sizeof(int), 0);
				log_info(ptrLog, "Se envio bytes");
			}
			free(bufferRespuesta);
			free(estructuraEnviarBytes->buffer);
			free(estructuraEnviarBytes);

			return 1;
		} else {
			log_error(ptrLog,
					"Se quiere ejecutar una operacion donde no hay una CPU o un Kernel");
			log_error(ptrLog, "Se envia la respuesta fallida");
			bufferRespuesta=malloc(sizeof(int));
			respuestaINT = -1;
			memcpy(bufferRespuesta, &respuestaINT, sizeof(int));
			enviarDatos(sock, &bufferRespuesta, sizeof(int), 0);
			free(estructuraEnviarBytes);
			free(estructuraEnviarBytes->buffer);
			free(bufferRespuesta);
			return 1;	//VERIFICAR
		}

	case 3:
		log_debug(ptrLog, "En el case (SERVIDOR)");
		log_debug(ptrLog, "Handshake");
		log_debug(ptrLog, "Tipo de cliente: %u",
				*(*package + sizeof(uint32_t)));
		//log_debug(ptrLog, "%u",
		//		*(*package + sizeof(uint32_t) + sizeof(uint32_t)));
		handshake(deserializarHandshake(package), tipoProceso);
		//Ejecutar funcion Handshake
		free(respuestaOperacion);
		return 1;

	case 4:
		log_debug(ptrLog, "En el case (SERVIDOR)");
		log_debug(ptrLog, "Cambio de proceso activo");
		//log_debug(ptrLog, "PID: %u", **package + sizeof(uint32_t));
		if (*tipoProceso == KERNEL || *tipoProceso == CPU) {
			log_info(ptrLog, "Se procede a cambiar el PID activo");
			cambiarProcesoActivo(deserializarCambiarProcesoActivo(package),
					procesoActivo);
			free(respuestaOperacion);
			return 1;

		} else {
			log_error(ptrLog,
					"Se quiere ejecutar una operacion donde no hay una CPU o un Kernel");
			free(respuestaOperacion);
			return 1;
		}

	case 5:
		log_debug(ptrLog, "En el case (SERVIDOR)");
		log_debug(ptrLog, "Crear segmento");
		log_debug(ptrLog, "PID %u", *(*package + sizeof(uint32_t)));
		//log_debug(ptrLog, "%u",
		//	*(*package + sizeof(uint32_t) + sizeof(uint32_t)));

		if (*tipoProceso == KERNEL) {
			log_info(ptrLog,
					"Se procede a atender la solicitud de crear segmento del kernel");
			respuestaINT = crearSegmento((deserializarCrearSegmento(package)));
			memcpy(respuestaOperacion, &respuestaINT, sizeof(int));

			//Envio tamaño de la respuesta de la funcion

			enviarDatos(sock, &respuestaOperacion, sizeof(int), 0);
			free(respuestaOperacion);
			return 1;
		} else {

			respuestaINT = -1;
			memcpy(respuestaOperacion, &respuestaINT, sizeof(int));
			enviarDatos(sock, &respuestaOperacion, sizeof(int), 0);
			log_error(ptrLog,
					"Se quiere ejecutar una operacion donde no hay una CPU o un Kernel");
			free(respuestaOperacion);
			return 1;
		}

	case 6:
		log_debug(ptrLog, "En el case (SERVIDOR)");
		log_debug(ptrLog, "Destruir segmento");
		log_debug(ptrLog, "PID %u", *(*package + sizeof(uint32_t)));
		if (*tipoProceso == KERNEL) {
			respuestaINT = destruirSegmento(
					deserializarDestruirSegmento(package));
			memcpy(respuestaOperacion, &respuestaINT, sizeof(int));
			enviarDatos(sock, &respuestaOperacion, sizeof(int), 0);
			free(respuestaOperacion);
			log_info(ptrLog, "Se realizo la destruccion del segmento");
			return 1;

		} else {
			respuestaINT = -1;
			memcpy(respuestaOperacion, &respuestaINT, sizeof(int));
			enviarDatos(sock, &respuestaOperacion, sizeof(int), 0);
			free(respuestaOperacion);
			log_error(ptrLog,
					"Se quiere ejecutar una operacion donde no hay una CPU o un Kernel");
			return 1;
		}
	default:
		log_debug(ptrLog, "En el case (SERVIDOR)");
		log_error(ptrLog, "No llego la opción correcta. OPERACION INCORRECTA.");
		return -1;
	}

}

t_solicitarBytes* deserializarSolicitarBytes(char** package) {
	t_solicitarBytes *solicitarBytes = malloc(sizeof(t_solicitarBytes));
	int offset = sizeof(uint32_t), tmp_size = 0;

/*	log_debug(ptrLog,
			"Dentro de la fn DeserealizarSolicitarBytes,antes de Deserializar (SERVIDOR) - imprimiendo el paquete");
	log_debug(ptrLog, "Operacion: %u", **package);
	log_debug(ptrLog, "Base: %u", *(*package + sizeof(uint32_t)));
	log_debug(ptrLog, "Offset: %u",
			*(*package + sizeof(uint32_t) + sizeof(uint32_t)));
	log_debug(ptrLog, "Tamanio: %u",
			*(*package + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t)));*/

	memcpy(&solicitarBytes->base, (*package + offset), tmp_size =
			sizeof(solicitarBytes->base));
	offset += tmp_size;
	memcpy(&solicitarBytes->offset, *package + offset, tmp_size =
			sizeof(solicitarBytes->offset));
	offset += tmp_size;
	memcpy(&solicitarBytes->tamanio, *package + offset, tmp_size =
			sizeof(solicitarBytes->tamanio));

	log_debug(ptrLog,
			"Dentro de la fn DeserealizarSolicitarBytes,despues de Deserializar (SERVIDOR) - Imprimiendo la estructura");
	log_debug(ptrLog, "Base %u ", solicitarBytes->base);
	log_debug(ptrLog, "Offset %u ", solicitarBytes->offset);
	log_debug(ptrLog, "Tamanio %u ", solicitarBytes->tamanio);

	return solicitarBytes;
}
t_enviarBytes* deserializarEnviarBytes(char**package) {

	//Inicializo las variables locales
	t_enviarBytes* enviarBytes = malloc(sizeof(t_enviarBytes));
	int offset = sizeof(uint32_t), tmp_size = 0;

	//Dejo los Logs
	/*log_debug(ptrLog,
			"Dentro de la fn DeserealizarEnviarBytes,antes de Deserializar (SERVIDOR) - imprimiendo el paquete");
	log_debug(ptrLog, "Operacion %u", **package);
	log_debug(ptrLog, "Base: %u", *(*package + sizeof(uint32_t)));
	log_debug(ptrLog, "Offset: %u",
			*(*package + sizeof(uint32_t) + sizeof(uint32_t)));
	log_debug(ptrLog, "Tamanio: %u",
			*(*package + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t)));
	log_debug(ptrLog, "%s",
	 *(*package + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t)
	 + sizeof(uint32_t)));*/

	//Deserializo
	memcpy(&enviarBytes->base, (*package + offset), tmp_size =
			sizeof(enviarBytes->base));
	offset += tmp_size;
	memcpy(&enviarBytes->offset, *package + offset, tmp_size =
			sizeof(enviarBytes->offset));
	offset += tmp_size;
	memcpy(&enviarBytes->tamanio, *package + offset, tmp_size =
			sizeof(enviarBytes->tamanio));
	offset += tmp_size;
	enviarBytes->buffer = calloc(enviarBytes->tamanio, sizeof(char));
	memcpy(enviarBytes->buffer, *package + offset,
			tmp_size = (enviarBytes->tamanio));
	//Dejo los logs dps de Deserializar
	log_debug(ptrLog,
			"Dentro de la fn DeserealizarEnviarBytes,despues de Deserializar (SERVIDOR) - Imprimiendo la estructura");
	log_debug(ptrLog, "Base %u ", enviarBytes->base);
	log_debug(ptrLog, "Offset %u ", enviarBytes->offset);
	log_debug(ptrLog, "Tamanio %u ", enviarBytes->tamanio);
	//log_debug(ptrLog, "%s ", enviarBytes->buffer);

	return enviarBytes;
}
t_handshake* deserializarHandshake(char** package) {

	//Inicializo las variables locales
	t_handshake* enviarHandshake = malloc(sizeof(t_handshake));
	int offset = sizeof(uint32_t), tmp_size = 0;
	//Dejo logs
	/*log_debug(ptrLog,
			"Dentro de la fn deserializarHandshake,antes de Deserializar (SERVIDOR) - imprimiendo el paquete");
	log_debug(ptrLog, "%u", *(*package + sizeof(uint32_t)));
	//log_debug(ptrLog, "%u", *(*package + sizeof(uint32_t) + sizeof(uint32_t)));
	//Deserializo*/
	memcpy(&enviarHandshake->programID, (*package + offset), tmp_size =
			sizeof(enviarHandshake->programID));
	offset += tmp_size;
	//memcpy(&enviarHandshake->tipo, *package + offset, tmp_size =
	//		sizeof(enviarHandshake->tipo));
	offset += tmp_size;
	//Dejo logs luego de deserializar
	log_debug(ptrLog,
			"Dentro de la fn deserializarHandshake,despues de Deserializar (SERVIDOR) - Imprimiendo la estructura");
	log_debug(ptrLog, "%u ", enviarHandshake->programID);
	//log_debug(ptrLog, "%u ", enviarHandshake->tipo);

	return enviarHandshake;
}
t_crear_segmento* deserializarCrearSegmento(char** package) {
	t_crear_segmento* crear_segmento = malloc(sizeof(t_crear_segmento));
	int offset = sizeof(uint32_t), tmp_size = 0;

	//Dejo logs
	/*log_debug(ptrLog,
			"Dentro de la fn deserializarCrearSegmento,antes de Deserializar (SERVIDOR) - imprimiendo el paquete");
	log_debug(ptrLog, "%u", *(*package + sizeof(uint32_t)));
	log_debug(ptrLog, "%u", *(*package + sizeof(uint32_t) + sizeof(uint32_t)));*/
	//Deserializo
	memcpy(&crear_segmento->programID, (*package + offset), tmp_size =
			sizeof(crear_segmento->programID));
	offset += tmp_size;
	memcpy(&crear_segmento->tamanio, *package + offset, tmp_size =
			sizeof(crear_segmento->tamanio));
	offset += tmp_size;
	//Dejo logs luego de deserializar
	log_debug(ptrLog,
			"Dentro de la fn deserializarCrearSegmento,despues de Deserializar (SERVIDOR) - Imprimiendo la estructura");
	log_debug(ptrLog, "PID: %u ", crear_segmento->programID);
	log_debug(ptrLog, "Tamanio: %u ", crear_segmento->tamanio);

	return crear_segmento;
}
t_destruir_segmento* deserializarDestruirSegmento(char** package) {
	t_destruir_segmento* destruir_segmento = malloc(
			sizeof(t_destruir_segmento));
	int offset = sizeof(uint32_t), tmp_size = 0;

	//Dejo logs
	/*log_debug(ptrLog,
			"Dentro de la fn deserializarDestruirSegmento,antes de Deserializar (SERVIDOR) - imprimiendo el paquete");
	log_debug(ptrLog, "%u", *(*package + sizeof(uint32_t)));
	//log_debug(ptrLog,"%u",*(*package+sizeof(uint32_t)));*/
	//Deserializo
	memcpy(&destruir_segmento->programID, (*package + offset), tmp_size =
			sizeof(destruir_segmento->programID));
	offset += tmp_size;

	//Dejo logs luego de deserializar
	log_debug(ptrLog,
			"Dentro de la fn deserializarDestruirSegmento,despues de Deserializar (SERVIDOR) - Imprimiendo la estructura");
	log_debug(ptrLog, "%u ", destruir_segmento->programID);

	return destruir_segmento;

}
t_cambio_proc_activo* deserializarCambiarProcesoActivo(char** package) {

	t_cambio_proc_activo* cambiar_proceso_activo = malloc(
			sizeof(t_cambio_proc_activo));
	int offset = sizeof(uint32_t), tmp_size = 0;

	//Dejo logs
	/*log_debug(ptrLog,
			"Dentro de la fn deserializarCambiarProcesoActivo,antes de Deserializar (SERVIDOR) - imprimiendo el paquete");
	log_debug(ptrLog, "%u", *(*package + sizeof(uint32_t)));
	//log_debug(ptrLog,"%u",*(*paccambioProcActivo.programID=86;kage+sizeof(uint32_t)));*/
	//Deserializo
	memcpy(&cambiar_proceso_activo->programID, (*package + offset), tmp_size =
			sizeof(cambiar_proceso_activo->programID));

	//Dejo logs luego de deserializar
	log_debug(ptrLog,
			"Dentro de la fn deserializarCambiarProcesoActivo,despues de Deserializar (SERVIDOR) - Imprimiendo la estructura");
	log_debug(ptrLog, "PID Activo: %u ", cambiar_proceso_activo->programID);

	return cambiar_proceso_activo;
}

