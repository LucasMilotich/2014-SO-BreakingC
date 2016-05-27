#include "SocketServerPLP.h"
#include "funcionesPLP.h"
#include <comun/Structs.h>

#define BUFF_SIZE 2048
#define BACKLOG 10
extern t_config* config;
extern t_log* logPlp;
extern sem_t semNuevoProg;
extern sem_t semMensajeImpresion;

t_list* listaClientesProgramas;	//lista con socket y pid correspondiente a cada cliente

void* comenzarSocketServer(void* socket) {

	/* Obtengo por archivo de configuracion el Puerto donde ingresan las conexiones nuevas,
	 * seteo el grado de multiprogramacion*/
	int puerto = config_get_int_value(config, "PUERTO_PROG");
	int socketServidor;					// Descriptor del socket servidor
	fd_set descriptoresLectura;			// Descriptores de interes para select()
	fd_set master;						// Descriptor master
	int maximo;							// Número de descriptor más grande
	int socketUMV = (int) socket;			// Socket de la conexion con la UMV
	int i;

	listaClientesProgramas = list_create();
	log_debug(logPlp, "Se abre el socket de conexion del servidor");
	socketServidor = Abrir_Conexion_Servidor(puerto);
	if (socketServidor == -1) {
		log_error(logPlp, "Error al abrir el servidor");
		exit(-1);
	}
	log_debug(logPlp, "Socket servidor abierto correctamente");

	// Se inicializa descriptoresLectura
	FD_ZERO(&descriptoresLectura);
	FD_ZERO(&master);

	//Se añade para select() el socket servidor
	FD_SET(socketServidor, &master);
	maximo = socketServidor;

	while (1) {

		descriptoresLectura = master;

		//Se pide el valor del descriptor más grande. Si no hay ningún cliente, devolverá 0
		/*maximo = dameMaximo(socketCliente, numeroClientes);

		 if (maximo < socketServidor)
		 maximo = socketServidor;*/

		/* Espera indefinida hasta que alguno de los descriptores tenga algo
		 * que decir: un nuevo cliente o un cliente ya conectado que envía un
		 * mensaje */
		log_debug(logPlp,
				"Esperando que los descriptores tengan algo que decir...");

		if (select(maximo + 1, &descriptoresLectura, NULL, NULL, NULL ) == -1) {
			log_error(logPlp, "Error en el Select");
			exit(-1);
		}

		/* Se comprueba si algún cliente ya conectado ha enviado algo */
		log_debug(logPlp,
				"Se comprueba si algún programa ya conectado ha enviado algo");
		for (i = 0; i <= maximo; i++) {
			if (FD_ISSET (i, &descriptoresLectura)) {
				if (i == socketServidor) {
					int newfd = Aceptar_Conexion_Cliente(socketServidor);
					if (newfd == -1) {
						log_error(logPlp, "Error al aceptar Programa");
					} else {
						log_debug(logPlp, "Se conecto un nuevo programa!!");
						FD_SET(newfd, &master);
						if (newfd > maximo) {/*Llevamos track del maximo*/
							maximo = newfd;
						}
					}
				} else {
					/* Se lee lo enviado por el cliente y se escribe en pantalla */
					char* buffer;
					if ((recibirDatos(i, &buffer, NULL )) <= 0) {
						bool _buscaCliente(t_socket_pid* unCliente) {
							return unCliente->socket == i;
						}
						t_socket_pid *unCliente = list_remove_by_condition(	listaClientesProgramas, (void*) _buscaCliente);
						//t_socket_pid *unCliente = list_find(	listaClientesProgramas, (void*) _buscaCliente);

						log_error(logPlp, "El programa:%d se ha desconectado",
								unCliente->pid);

						if (unCliente->terminado == false) {
							pcbAFinalizar = unCliente->pid;
							sem_post(&semProgramaFinaliza);
						}
						free(unCliente);
						//list_remove_and_destroy_by_condition(listaClientesProgramas,(void*)_buscaCliente,free);
						close(i);
						FD_CLR(i, &master);/*Removemos el socket del cliente del master set*/
						//free(buffer);
					} else {
						log_debug(logPlp, "Cliente %d envía %s", i, buffer);
						t_pcb *unPcb;
						unPcb = crearPCB(buffer, socketUMV);
						t_socket_pid* unCliente = malloc(sizeof(t_socket_pid));/*Definimos la estructura del cliente con su socket y pid asociado*/
						unCliente->pid = nroProg;
						unCliente->socket = i;
						unCliente->terminado = false;
						list_add(listaClientesProgramas, unCliente);
						if (unPcb == NULL && buffer[0] == -1) {
							log_error(logPlp,
									"Hubo un fallo en la creacion del PCB - Causa: Memory Overload");
							t_imprimibles *unMensaje = malloc(
									sizeof(t_imprimibles));
							unCliente->terminado = true;
							char* texto =
									"Error al crear el pcb correspondiente a tu programa - Causa: Memory Overload";
							unMensaje->PCB_ID = nroProg;
							unMensaje->tipoDeValor = ERROR;
							unMensaje->valor = texto;
							queue_push(colaImprimibles, unMensaje);
							sem_post(&semMensajeImpresion);
						} else if (unPcb == NULL && buffer[0] == -2) {
							log_error(logPlp,
									"Hubo un fallo en la creacion del PCB - Causa: Segmentation Fault");
							t_imprimibles *unMensaje = malloc(
									sizeof(t_imprimibles));
							unCliente->terminado = true;
							char* texto =
									"Error al crear el pcb correspondiente a tu programa - Causa: Segmentation Fault";
							unMensaje->PCB_ID = nroProg;
							unMensaje->tipoDeValor = ERROR;
							unMensaje->valor = texto;
							queue_push(colaImprimibles, unMensaje);
							sem_post(&semMensajeImpresion);
						} else {
							log_debug(logPlp,
									"Todo reservado, se procede a ponerlo en la lista New");
							list_add(colaNew, unPcb);
							log_info(logPlp,
									"===== Lista de Procesos en cola NEW =====");
							void _imprimirProcesoNew(t_pcb* pcb) {
								log_debug(logPlp, "Proceso %d en estado NEW \n",
										pcb->pcb_id);
							}
							list_iterate(colaNew, (void*) _imprimirProcesoNew);
							sem_post(&semNuevoProg);
						}
						free(buffer);
					}
				}
			}
		}
	}

	close(socketServidor);
	close(socketUMV);
	return 0;
}
