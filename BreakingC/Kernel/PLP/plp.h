#ifndef PLP_H_
#define PLP_H_
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>
#include "../kernel.h"

//#define RUTA_ARCHIVO_LOG_PLP		"/home/utnso/tp-2014-1c-breaking-c/BreakingC/Kernel_2/PLP/log_plp.txt"

void* vaciarColaExit();/* Gestiona el paso de los pcbs de New a Ready*/
void* comenzarSocketServer(); /* Abre el servidor por donde van a llegar las conexiones de los programas*/
void* mensajesPrograma();

typedef struct {
	int socket;
	int32_t pid;
	bool terminado;
}t_socket_pid;

#endif /* PLP_H_ */
