/*
 * SocketServerPLP.h
 *
 *  Created on: 19/06/2014
 *      Author: utnso
 */

#ifndef SOCKETSERVERPLP_H_
#define SOCKETSERVERPLP_H_
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "plp.h"
#include <comun/UMV_Structs.h>
#include "../kernel.h"
#include <sockets/Socket_Servidor.h>
#include <sockets/Socket.h>

void socketServidor();
void* comenzarSocketServer();


#endif /* SOCKETSERVERPLP_H_ */
