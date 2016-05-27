/* Franco Bustos.
 *
 * Funciones para abrir/establecer sockets de un cliente con un servidor.
 *
 */


/*
* Includes del sistema
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

/*
* Conecta con un servidor remoto a traves de socket INET
*/
int Abrir_Conexion (char *ip, int puerto)
{
	struct sockaddr_in socket_info;
	int unSocket;

	// Se carga informacion del socket
	socket_info.sin_family = AF_INET;
	socket_info.sin_addr.s_addr = inet_addr(ip);
	socket_info.sin_port = htons(puerto);
	
	// Crear un socket:
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	unSocket = socket (AF_INET, SOCK_STREAM, 0);
	if (unSocket < 0)
		return -1;

	// Conectar el socket con la direccion 'socketInfo'.
	if (connect (
			unSocket,
			(struct sockaddr *)&socket_info,
			sizeof (socket_info)) != 0)
	{
		return -1;
	}

	return unSocket;
}
