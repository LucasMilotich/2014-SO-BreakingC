/*
* Franco Bustos.
*
* Funciones para la apertura de un socket servidor y la conexion con sus
* clientes
*
*/

/* Includes del sistema */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

/*
* Se le pasa un socket de servidor y acepta en el una conexion de cliente.
* devuelve el descriptor del socket del cliente o -1 si hay problemas.
* Esta funcion vale para socket AF_INET o AF_UNIX.
*/
int Aceptar_Conexion_Cliente (int socket)
{
	socklen_t longitud_Cliente;
	struct sockaddr cliente;
	int socket_nuevaConexion;

	/*
	* La llamada a la funcion accept requiere que el parametro 
	* Longitud_Cliente contenga inicialmente el tamano de la
	* estructura Cliente que se le pase. A la vuelta de la
	* funcion, esta variable contiene la longitud de la informacion
	* util devuelta en Cliente
	*/
	longitud_Cliente = sizeof (cliente);
	socket_nuevaConexion = accept (socket, &cliente, &longitud_Cliente);
	if (socket_nuevaConexion < 0)
		return -1;

	/*
	* Se devuelve el descriptor en el que esta "enchufado" el cliente.
	*/
	return socket_nuevaConexion;
}

/*
* Abre un socket servidor de tipo AF_INET. Devuelve el descriptor
*	del socket o -1 si hay probleamas
* Se pasa como parametro el nombre del servicio. Debe estar dado
* de alta en el fichero /etc/services
*/
int Abrir_Conexion_Servidor (int puerto)
{
	struct sockaddr_in socketInfo;
	int socketEscucha;
	int optval = 1;

	/*
	// Crear un socket:
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	*/
	socketEscucha = socket (AF_INET, SOCK_STREAM, 0);
	if (socketEscucha == -1)
	 	return -1;

	// Hacer que el SO libere el puerto inmediatamente luego de cerrar el socket.
	setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &optval,
			sizeof(optval));
	/*
	* Se rellenan los campos de la estructura Direccion, necesaria
	* para la llamada a la funcion bind()
	*/
	socketInfo.sin_family = AF_INET;
	socketInfo.sin_port = htons(puerto);
	socketInfo.sin_addr.s_addr = INADDR_ANY;
	if (bind (
			socketEscucha,
			(struct sockaddr *)&socketInfo,
			sizeof (socketInfo)) != 0)
	{
		close (socketEscucha);
		return -1;
	}

	/*
	* Se avisa al sistema que comience a atender llamadas de clientes
	*/
	if (listen (socketEscucha, 10) == -1)
	{
		close (socketEscucha);
		return -1;
	}

	/*
	* Se devuelve el descriptor del socket servidor
	*/
	return socketEscucha;
}
