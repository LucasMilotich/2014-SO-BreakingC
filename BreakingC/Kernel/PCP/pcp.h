/*
 * pcp.h
 *
 *  Created on: 20/05/2014
 *      Author: utnso
 */

#ifndef PCP_H_
#define PCP_H_
#include "../kernel.h"
#include <stdbool.h>
#include <semaphore.h>
#include "operacionesPCP.h"
//#define RUTA_ARCHIVO_LOG_PCP		"/home/utnso/tp-2014-1c-breaking-c/BreakingC/Kernel/PCP/log_pcp.txt"
extern t_log *log_pcp;


extern t_log *log_pcp;
extern sem_t semClienteOcioso;
extern pthread_mutex_t mutex;

void* hiloPorIO(void*entradaSalida);
void *hiloClienteOcioso(void);
void *comienzaPCP(void);
void* hiloPCBaFinalizar(void);
void server_cpu(void);
void nuevoCliente (int);
void inicializaciones(int* nroDescriptorMax, int socketServidor);
void envioPCBaClienteOcioso(t_clienteCPU *clienteSeleccionado);
void comprobarMensajesDeClientes(void);
void comprobarClientesNuevos(int socketServidor);
void cerrarConexionCliente(t_clienteCPU *);
int cuantosClientesSinPCB(void);


void cargoVarCompartidasEnLista(void);
void imprimoListaVarCompartida(void);

void cargoSemaforosEnLista(void);
void imprimoListaSemaforos(void);

void cargoIOenLista(void);
void imprimoListaIO(void);

void imprimoListaBloqueados(void);

void imprimoInformacionGeneral(void);
void recorrerUnaColaDePcbs(t_queue*);
void encolarPCB_IO(t_pcb *);

/*
void operacionImprimirValor (uint32_t ,char*);
void operacionImprimirTexto (uint32_t ,char*);
void operacionImprimirError (uint32_t ,char*);
*/
#endif /* PCP_H_ */
