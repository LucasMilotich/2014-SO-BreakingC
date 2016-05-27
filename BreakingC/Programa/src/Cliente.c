#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <sockets/Socket.h>
#include <sockets/Socket_Cliente.h>
#include <comun/Structs.h>
#include <stdbool.h>

extern t_log* Log;
extern t_config* config;

int EnviarProgramaAlKernel(char *programa) {
	int unSocket;
	char *direccion = config_get_string_value(config, "IP");
	int puerto = config_get_int_value(config, "PUERTO");
	char* server_reply;
	char operacion;
	int valor = 0;
	bool salir = false;

	log_debug(Log, "Conectando con el kernel..");

	if ((unSocket = Abrir_Conexion(direccion, puerto)) < 0) {
		log_error(Log, "Error al crear socket");
		return EXIT_FAILURE;
	}

	log_debug(Log, "Conectado!");

	// Se envia por socket el programa al PLP (con el caracter \0 al final).
	log_debug(Log, "Enviando programa al PLP..");
	if (enviarDatos(unSocket, &programa, strlen(programa) + 1, NOTHING) >= 0) {
		log_debug(Log, "Programa enviado");
	} else {
		log_error(Log, "Error al enviar datos. Servidor no encontrado.");
		return EXIT_FAILURE;
	}

	// Respuesta del Servidor
	while (1)
	{
		if (recibirDatos(unSocket, &server_reply, &operacion) < 0)
		{
			log_error(Log, "Error al recibir datos del servior.");
			return EXIT_FAILURE;
		}

		switch(operacion)
		{
			case IMPRIMIR_TEXTO:
				printf("%s\n", server_reply);
				break;
			case IMPRIMIR_VALOR:
				memcpy(&valor, server_reply, 4);
				printf("%d\n", valor);
				break;
			default:
				printf("%s\n", server_reply);
				salir = true;
				break;
		}

		free(server_reply);
		if (salir) break;
	}

	finalizarConexion(unSocket);

	return EXIT_SUCCESS;

}
