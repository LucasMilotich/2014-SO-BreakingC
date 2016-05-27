/*
 * Franco Bustos.
 *
 * Funciones para que un cliente pueda abrir sockets con un servidor.
 */
#ifndef _SOCKET_CLIENTE_H
#define _SOCKET_CLIENTE_H

#include <sys/socket.h>

/**
 * Abre un socket INET con un servidor que est� corriendo en Host_Servidor y que atienda
 * al servicio cuyo nombre es Servicio. 
 * Host_Servidor debe estar dado de alta en /etc/hosts.
 * Servicio debe estar dado de alta en /etc/services como tcp.
 */
int Abrir_Conexion (char *ip, int puerto);

#endif
