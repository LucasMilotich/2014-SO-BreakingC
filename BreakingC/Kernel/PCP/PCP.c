#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <string.h>
#include <comun/Structs.h>
#include "../kernel.h"
#include "pcp.h"
#include <semaphore.h>
#include <commons/string.h>

#include <sys/wait.h>

t_config *config;
t_list * listaClientes, *listaVarCompartidas, *listaSemaforos, *listaIO,
*listaBloqueados;
t_log* log_pcp;
sem_t semClienteOcioso;

t_EstructuraInicial *estructuraInicial;	// Aca guardo el Quantum y el retardoQuantum cargado desde archivo de configuracion

int quantumActual;			// Quantum cargado desde archivo de Configuracion
int retardoQuantum;			// retardoQuantum cargado desde archivo de config
int cpuAcerrar;		 		// CPU que se va a cerrar
fd_set descriptoresLectura;			// Descriptores de interes para select()
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_exit;

void *comienzaPCP() {
	listaVarCompartidas = list_create(); // Lista de las variables compartidas
	listaSemaforos = list_create(); // Lista de Semaforos
	listaIO = list_create(); // Lista de los Dispositivos IO
	listaBloqueados = list_create(); //Lista de procesos bloqueados por semaforo
	config = config_create(getenv("CONFIG_KERNEL"));
	log_pcp = creacionLog(config_get_string_value(config, "LOG_KERNEL"), "PCP");
	log_debug(log_pcp, "Inicia el proceso PCP..");
	log_trace(log_pcp, "Creado log del PCP");
	log_trace(log_pcp, "Cargado el archivo de configuracion");

	// ######################## Cargo Archivo de config ######################## //
	// Cargo QUANTUM y retardo Quantum en estructuraInicial
	estructuraInicial = malloc(sizeof(t_EstructuraInicial));
	estructuraInicial->Quantum = config_get_int_value(config, "QUANTUM");
	estructuraInicial->RetardoQuantum = config_get_int_value(config, "RETARDO");
	log_trace(log_pcp, "Cargado el Quantum: %d , retardoQuantum:%d",
			estructuraInicial->Quantum, estructuraInicial->RetardoQuantum);
	cpuAcerrar=0;
	// Cargo las varCompartidas en listaVarCompartidas asi es mas comodo manejarlas
	cargoVarCompartidasEnLista();
	// Cargo los semaforos en listaSemaforos
	cargoSemaforosEnLista();
	// Cargo los IO en listaIO
	cargoIOenLista();
	pthread_t hiloIO, hiloPCB;					//hilo por IO y PCB
	sem_init(&semClienteOcioso, 1, 0);		// Semaforo para clientes ociosos

	//Creo hilo por cada dispositivo
	void _creoHilos(void* entradaSalida) {
		if (pthread_create(&hiloIO, NULL, hiloPorIO, (void*) entradaSalida)
				!= 0) {
			perror("could not create thread");
		}
	}

	list_iterate(listaIO, (void*) _creoHilos);

	// creo hilo para manejar programas que se cierren inesperadamente
	if (pthread_create(&hiloPCB, NULL, (void*) hiloPCBaFinalizar, NULL ) != 0) {
		perror("could not create thread");
	}
	// ######################## 	Fin loading		 ######################## //

	server_cpu();

	config_destroy(config);
	log_destroy(log_pcp);
	list_destroy_and_destroy_elements(listaVarCompartidas, free);
	list_destroy_and_destroy_elements(listaIO, free);
	list_destroy_and_destroy_elements(listaSemaforos, free);
	return 0;
}
void *hiloPorIO(void* inputOutput) {
	t_IO *entradaSalida = (t_IO*) inputOutput;
	t_solicitudes *solicitud;
	//solicitud = malloc(sizeof(t_solicitudes));
	log_debug(log_pcp, "%s: Hilo creado, (ID:%d, colaSize:%d)",
			entradaSalida->nombre, entradaSalida->ID_IO,
			queue_size(entradaSalida->colaSolicitudes));
	while (1) {
		//aca el if estaria de mas, despues sacarlo
		if (queue_is_empty(entradaSalida->colaSolicitudes)) {
			log_debug(log_pcp, "%s: Me quedo a la espera de solicitudes",
					entradaSalida->nombre);
			sem_wait(&entradaSalida->semaforo);
			log_debug(log_pcp,
					"\nPase semaforo entradaSalidaSemaforo ID:%s\n",
					entradaSalida->nombre);

		} else {
			solicitud = queue_pop(entradaSalida->colaSolicitudes);
			log_trace(log_pcp, "%s: Solicitud recibida, valor:%d",
					entradaSalida->nombre, solicitud->valor);
			log_trace(log_pcp, "%s: Hago retardo de :%d milisegundos",
					entradaSalida->nombre,
					solicitud->valor * entradaSalida->valor);
			sleep(solicitud->valor * entradaSalida->valor / 1000);
			queue_push(colaReady, solicitud->pcb);
			sem_post(&semNuevoPcbColaReady);
			log_debug(log_pcp, "%s: Envio PCB ID:%d a colaReady",
					entradaSalida->nombre, solicitud->pcb->pcb_id);
			free(solicitud);
		}
	}
	return 0;
}
void *hiloClienteOcioso() {

	while (1) {
		// Tengo la colaReady con algun PCB, compruebo si tengo un cliente ocioso..

		sem_wait(&semNuevoPcbColaReady);
		log_debug(log_pcp, "\nPase semaforo semNuevoPcbColaReady\n");
		sem_wait(&semClienteOcioso);
		log_debug(log_pcp, "\nPase semaforo semClienteOcioso\n");
		//if (list_size(list_filter(listaClientes, (void*) _sinPCB), 0) >0) {
		bool _sinPCB(t_clienteCPU * unCliente) {
			//return unCliente->pcbAsignado.pcb_id==-1;
			return unCliente->fueAsignado == false;
		}

		t_clienteCPU *clienteSeleccionado = list_get(list_filter(listaClientes, (void*) _sinPCB), 0);
		envioPCBaClienteOcioso(clienteSeleccionado);

		//}
	}
	return 0;
}
void* hiloPCBaFinalizar() {
	while (1) {
		sem_wait(&semProgramaFinaliza);
		log_debug(log_pcp, "Debido a que se detuvo el programa %d, se envia el pcb a la cola de Exit", pcbAFinalizar);

		bool obtenerDeBloqueados(t_pcbBlockedSemaforo* pcbBlocked) {
			return pcbBlocked->pcb->pcb_id == pcbAFinalizar;
		}

		bool obtenerDeReady(t_pcb* pcbReady) {
			return pcbReady->pcb_id == pcbAFinalizar;
		}

		bool obtenerDeClientes(t_clienteCPU* cliente) {
			return cliente->pcbAsignado->pcb_id == pcbAFinalizar;
		}

		while (1) {

			t_pcbBlockedSemaforo* pcbBlock = list_remove_by_condition(listaBloqueados, (void*) obtenerDeBloqueados);
			if (pcbBlock != NULL ) {
				queue_push(colaExit, pcbBlock->pcb);
				sem_post(&semProgExit);
				sem_post(&semMultiProg);
				pthread_mutex_unlock(&mutex_exit);
				log_debug(log_pcp, "El pcb se encontraba en la cola de bloqueados");
				break;
			}

			t_pcb* pcb = list_remove_by_condition(colaReady->elements, (void*) obtenerDeReady);
			if (pcb != NULL ) {
				queue_push(colaExit, pcb);
				sem_post(&semProgExit);
				sem_post(&semMultiProg);
				pthread_mutex_unlock(&mutex_exit);
				log_debug(log_pcp, "El pcb se encontraba en la cola de ready");
				break;
			}

			t_clienteCPU* cliente = list_find(listaClientes, (void*) obtenerDeClientes);
			if (cliente != NULL ) {
				cliente->programaCerrado = true;
				cliente->fueAsignado=false;
				queue_push(colaExit, cliente->pcbAsignado);
				sem_post(&semProgExit);
				sem_post(&semMultiProg);
				pthread_mutex_unlock(&mutex_exit);
				log_debug(log_pcp, "El programa se encontraba en ejecucion");
				break;
			}

			sleep(2);
		}
	}
	return 0;
}

void server_cpu() {
	int puerto = config_get_int_value(config, "PUERTO_CPU");
	int socketServidor;					// Descriptor del socket servidor
	listaClientes = list_create();	// Lista de los clientes(CPUS) en sistema
	int *maximum;						// Numero de descriptor mas grande
	maximum = malloc(sizeof(int));

	log_debug(log_pcp, "Se abre el socket de conexion del servidor");
	socketServidor = Abrir_Conexion_Servidor(puerto);
	if (socketServidor == -1) {
		log_error(log_pcp, "Error al abrir el servidor");
		exit(-1);
	}
	log_debug(log_pcp, "Socket servidor abierto correctamente");
	pthread_t hiloDEClienteOcioso;
	if (pthread_create(&hiloDEClienteOcioso, NULL, (void*) hiloClienteOcioso,
			NULL ) != 0) {
		perror("could not create thread");
	} else {
		log_debug(log_pcp, "Hilo cliente ocioso creado");
	}

	// Bucle infinito para futuros clientes y msjes que manden viejos clientes
	while (1) {
		inicializaciones(maximum, socketServidor);
		// Espera indefinida hasta que alguno de los descriptores tenga algo
		// que decir: un nuevo cliente o un cliente ya conectado que envía un
		//mensaje
		imprimoInformacionGeneral();
		log_debug(log_pcp,
				"Esperando que los descriptores tengan algo que decir...");
		select(*maximum + 1, &descriptoresLectura, NULL, NULL, NULL );
		// Se comprueba si algún cliente ya conectado envio algo
		comprobarMensajesDeClientes();
		// Se comprueba si algún cliente nuevo desea conectarse y se agrega a listaClientes
		comprobarClientesNuevos(socketServidor);

		//imprimoInformacionGeneral();
		imprimoListaVarCompartida();
		imprimoListaSemaforos();
		imprimoListaIO();
	}
	finalizarConexion(socketServidor);
}
void comprobarClientesNuevos(int socketServidor) {
	if (FD_ISSET (socketServidor, &descriptoresLectura))
		nuevoCliente(socketServidor);
}
void nuevoCliente(int servidor) {
	t_clienteCPU *client;

	char * estructuraInicialEnBuffer = malloc(sizeof(t_EstructuraInicial));
	client = malloc(sizeof(t_clienteCPU));
	client->socket = Aceptar_Conexion_Cliente(servidor);
	client->ID = (uint32_t) list_size(listaClientes);
	client->fueAsignado = false;
	client->programaCerrado = false;
	list_add(listaClientes, client);

	estructuraInicialEnBuffer = serializar_EstructuraInicial(estructuraInicial);
	enviarDatos(client->socket, &estructuraInicialEnBuffer,
			sizeof(t_EstructuraInicial), NOTHING);
	log_debug(log_pcp, "Cliente %d aceptado", client->ID);
	log_debug(log_pcp, "Quantum y retardoQuantum enviado al cliente %d",
			client->ID);
	sem_post(&semClienteOcioso);
	// Hago free del buffer
	//free(quantumEnBuffer);
	free(estructuraInicialEnBuffer);
	return;
}

void inicializaciones(int* maximum, int socketServidor) {
	// Se inicializa descriptoresLectura
	FD_ZERO(&descriptoresLectura);

	//Se añade para select() el socket servidor
	FD_SET(socketServidor, &descriptoresLectura);

	// Se añaden los sockets con los clientes ya
	// conectados que queremos despertarlos con select()
	void _hagoFD_SET(t_clienteCPU * unCliente) {
		FD_SET(unCliente->socket, &descriptoresLectura);
	}
	list_iterate(listaClientes, (void*) _hagoFD_SET);

	//Se pide el valor del descriptor más grande. Si no hay clientes, devuelve 0
	if (list_size(listaClientes) > 0) {
		void _dameMaximo(t_clienteCPU * unCliente) {
			if (unCliente->socket > *maximum) {
				*maximum = unCliente->socket;
			}
		}
		list_iterate(listaClientes, (void*) _dameMaximo);
	} else {
		*maximum = 0;
	}
	if (*maximum < socketServidor) {
		*maximum = socketServidor;
	}

}
void envioPCBaClienteOcioso(t_clienteCPU *clienteSeleccionado) {
	imprimoInformacionGeneral();
	log_debug(log_pcp,
			"Obtengo un cliente ocioso que este esperando por un PCB... cliente:%d",
			clienteSeleccionado->ID);
	pthread_mutex_lock(&mutex);
	t_pcb *unPCB = queue_pop(colaReady);
	pthread_mutex_unlock(&mutex);
	log_debug(log_pcp, "Obtengo PCB_ID %d de la colaReady", unPCB->pcb_id);
	char *pcbSerializado;
	log_debug(log_pcp, "Serializo el PCB");
	pcbSerializado = serializar_pcb(unPCB);
	enviarDatos(clienteSeleccionado->socket, &pcbSerializado, sizeof(t_pcb),
			NOTHING);
	log_debug(log_pcp, "PCB enviado al CPU %d", clienteSeleccionado->ID);
	//clienteSeleccionado->pcbIDasignado = unPCB->pcb_id;
	clienteSeleccionado->pcbAsignado = unPCB;
	clienteSeleccionado->fueAsignado = true;
	imprimoInformacionGeneral();
}
void comprobarMensajesDeClientes(void) {
	char* buffer, operacion;
	log_debug(log_pcp, "Comprobando nuevas actividades...");
	void _comprueboReciboMensajes(t_clienteCPU *unCliente) {
		if (FD_ISSET (unCliente->socket, &descriptoresLectura)) {
			if ((recibirDatos(unCliente->socket, &buffer, &operacion)) > 0) {
				// Valido lo que me envio el cpu
				if (unCliente->programaCerrado && (operacion == QUANTUM || operacion == EXIT || operacion == IO || operacion == ERROR)){
					unCliente->fueAsignado = false;
					unCliente->programaCerrado = false;
					sem_post(&semClienteOcioso);
					return;
				}
				if (unCliente->programaCerrado && (operacion == IMPRIMIR_VALOR || operacion == IMPRIMIR_TEXTO || operacion == ASIG_VAR_COMPARTIDA || operacion == SIGNAL))
					return;

				switch (operacion) {
				case QUANTUM:
					operacionQuantum(unCliente, buffer);
					break;
				case IO:
					operacionIO(unCliente, buffer);
					break;
				case EXIT:
					operacionExit(unCliente, buffer);
					sem_post(&semMultiProg);
					break;
				case ERROR:
					operacionError(unCliente, buffer);
					sem_post(&semMultiProg);
					break;
				case IMPRIMIR_VALOR:
					operacionImprimir(IMPRIMIR_VALOR,
							unCliente->pcbAsignado->pcb_id, buffer);
					break;
				case IMPRIMIR_TEXTO:
					operacionImprimir(IMPRIMIR_TEXTO, unCliente->pcbAsignado->pcb_id,
							buffer);
					break;
				case LEER_VAR_COMPARTIDA:
					operacionesConVariablesCompartidas(LEER_VAR_COMPARTIDA,
							buffer, unCliente->socket);
					break;
				case ASIG_VAR_COMPARTIDA:
					operacionesConVariablesCompartidas(ASIG_VAR_COMPARTIDA,
							buffer, unCliente->socket);
					break;
				case WAIT:
					operacionesConSemaforos(WAIT, buffer, unCliente);
					break;
				case SIGNAL:
					operacionesConSemaforos(SIGNAL, buffer, unCliente);
					break;
				case SIGUSR:
					cpuAcerrar=unCliente->socket;
					free(buffer);
					break;
				}
			} else {
				// Se indica que el cliente ha cerrado la conexión
				cerrarConexionCliente(unCliente);
			}
		}
	}
	list_iterate(listaClientes, (void*) _comprueboReciboMensajes);
}
void cerrarConexionCliente(t_clienteCPU *unCliente) {
	log_debug(log_pcp, "CPU %d ha cerrado la conexión", unCliente->ID);
	_Bool _sacarCliente(t_clienteCPU * elCliente) {
		return unCliente->socket == elCliente->socket;
	}
	// Si tenia un pcb asignado, lo mando a la cola d exit para liberar recursos..
	if (unCliente->fueAsignado == true && cpuAcerrar!=unCliente->socket) {
		log_debug(log_pcp, "Agregado a la colaExit el PCB del pid: %d",
				unCliente->pcbAsignado->pcb_id);
		queue_push(colaExit, unCliente->pcbAsignado);
		sem_post(&semProgExit);
		pthread_mutex_unlock(&mutex_exit);
	}
	//list_remove_and_destroy_by_condition(listaClientes, (void*) _sacarCliente,	free);
	list_remove_by_condition(listaClientes,(void*)_sacarCliente);
	if (cpuAcerrar==unCliente->socket) { cpuAcerrar=0;	}
	else {sem_wait(&semClienteOcioso);}
	//list_remove_by_condition(listaClientes, (void*) _sacarCliente);
}

