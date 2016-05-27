/**
 * Franco Bustos.
 *
 * Funciones de lectura y escritura de la libreria de sockets.
 *
 */
#ifndef _SOCKET_H
#define _SOCKET_H

#include <sys/socket.h>
#include <stdint.h>

/** Lee Datos de tama�o Longitud de un socket cuyo descriptor es fd.
 * Devuelve el numero de bytes leidos o -1 si ha habido error
 * datos = buffer leido
 * op = operacion a realizar*/
int recibirDatos(int fd, char** Datos, char* op);

/** Envia Datos de tama�o Longitud por el socket cuyo descriptor es fd.
 * Devuelve el n�mero de bytes escritos o -1 si ha habido error.
 * Datos = buffer a enviar
 * tamanio = longitud del buffer
 * op = numero que indica el tipo de operacion a realizar */
int enviarDatos(int fd, char** Datos, uint32_t tamanio, char op);

//Cierra la conexion del socket
int finalizarConexion(int socket);
#endif
