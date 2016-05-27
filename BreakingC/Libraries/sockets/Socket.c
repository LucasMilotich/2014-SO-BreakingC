/*
 * Franco Bustos.
 *
 * Funciones de lectura y escritura en sockets
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <string.h>

#include "Socket.h"

int leer(int socket, char* buffer, int longitud);
int escribir(int socket, char* buffer, int longitud);

/*
 * Lee datos del socket. Devuelve el numero de bytes leidos o
 * 0 si se cierra fichero o -1 si hay error.
 */
int recibirDatos(int socket, char** datos, char* op) {
	int leido = 0;
	int longitud = sizeof(uint32_t);
	char* buffer = malloc(longitud + 1);

	/*
	 * Comprobacion de que los parametros de entrada son correctos
	 */
	if ((socket < 0) || (buffer == NULL ))
		return -1;

	/* HEADER
	 * Se reciben primero los datos necesarios que dan informacion
	 * sobre el verdadero buffer a recibir
	 */
	if ((leido = leer(socket, buffer, longitud + sizeof(char))) > 0) {
		if (op == NULL ) {
			memcpy(&longitud, buffer + 1, sizeof(uint32_t));
		} else {
			memcpy(op, buffer, sizeof(char));
			memcpy(&longitud, buffer + 1, sizeof(uint32_t));
		}
		free(buffer);
		buffer = malloc(longitud);
	} else {
		free(buffer);
		return -1;
	}

	/*
	 * Se recibe el buffer con los datos
	 */
	if ((leido = leer(socket, buffer, longitud)) < 0) {
		free(buffer);
		return -1;
	}

	if (leido != longitud)
		printf("No se han podido leer todos los datos del socket!!");

	*datos = (char*) malloc(longitud);
	memcpy(*datos, buffer, longitud);
	free(buffer);

	/*
	 * Se devuelve el total de los caracteres leidos
	 */
	return leido;
}

/*
 * Escribe dato en el socket cliente. Devuelve numero de bytes escritos,
 * o -1 si hay error.
 */
int enviarDatos(int socket, char** datos, uint32_t tamanio, char op) {
	int escrito = 0;

	/*
	 * Comprobacion de los parametros de entrada
	 */
	if ((socket == -1) || (*datos == NULL )|| (tamanio < 1))return -1;

	/* HEADER
	 * Se envian primero los datos necesarios que dan informacion
	 * sobre el verdadero buffer a enviar
	 */
	char* buffer = malloc(tamanio + sizeof(char)+sizeof(uint32_t)); //1B de op y 4B de long
	memcpy(buffer, &op, sizeof(char));
	memcpy(buffer + sizeof(char), &tamanio, sizeof(uint32_t));
	memcpy(buffer + sizeof(char)+sizeof(uint32_t), *datos, tamanio);

	/*
	 * Envio de Buffer [tamanio B]
	 */
	escrito = escribir(socket, buffer, tamanio + sizeof(char)+sizeof(uint32_t));
	free(buffer);

	/*
	 * Devolvemos el total de caracteres leidos
	 */
	return escrito;
}

//Finaliza la conexion de un socket
int finalizarConexion(int socket) {
	close(socket);
	return 0;
}

int escribir(int socket, char* buffer, int longitud) {
	int escrito = 0, aux = 0;
	/*
	 * Bucle hasta que hayamos escrito todos los caracteres que nos han
	 * indicado.
	 */
	while (escrito < longitud && escrito != -1) {
		aux = send(socket, buffer + escrito, longitud - escrito, 0);
		if (aux > 0) {
			/*
			 * Si hemos conseguido escribir caracteres, se actualiza la
			 * variable Escrito
			 */
			escrito = escrito + aux;
		} else {
			/*
			 * Si se ha cerrado el socket, devolvemos el numero de caracteres
			 * leidos.
			 * Si ha habido error, devolvemos -1
			 */
			if (aux == 0)
				return escrito;
			else
				switch (errno) {
				case EINTR:
				case EAGAIN:
					usleep(100);
					break;
				default:
					escrito = -1;
				}
		}
	}
	return escrito;
}

int leer(int socket, char* buffer, int longitud) {
	int leido = 0, aux = 0;
	/*
	 * Se recibe el buffer con los datos
	 * Mientras no hayamos leido todos los datos solicitados
	 */
	while (leido < longitud && leido != -1) {
		aux = recv(socket, buffer + leido, longitud - leido, 0);
		if (aux > 0) {
			/*
			 * Si hemos conseguido leer datos, incrementamos la variable
			 * que contiene los datos leidos hasta el momento
			 */
			leido = leido + aux;
		} else {
			/*
			 * Si read devuelve 0, es que se ha cerrado el socket. Devolvemos
			 * los caracteres leidos hasta ese momento
			 */
			if (aux == 0)
				break;
			if (aux == -1) {
				/*
				 * En caso de error, la variable errno nos indica el tipo
				 * de error.
				 * El error EINTR se produce si ha habido alguna
				 * interrupcion del sistema antes de leer ningun dato. No
				 * es un error realmente.
				 * El error EGAIN significa que el socket no esta disponible
				 * de momento, que lo intentemos dentro de un rato.
				 * Ambos errores se tratan con una espera de 100 microsegundos
				 * y se vuelve a intentar.
				 * El resto de los posibles errores provocan que salgamos de
				 * la funcion con error.
				 */
				switch (errno) {
				case EINTR:
				case EAGAIN:
					usleep(100);
					break;
				default:
					leido = -1;
				}
			}
		}
	}
	return leido;
}
