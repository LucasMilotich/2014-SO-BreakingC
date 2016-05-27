#include <stdio.h>
#include "plp.h"
#include "funcionesPLP.h"
#include "SocketServerPLP.h"

t_log* logPlp;
extern t_config* config;
pthread_mutex_t mutex_exit;

void *comienzaPLP(void) {

	int socketUMV;
	pthread_t threadSocket;
	pthread_t threadExit;
	pthread_t threadMensajeProg;
	pthread_mutex_init(&mutex_exit, NULL );
	config = config_create(getenv("CONFIG_KERNEL"));
	logPlp = creacionLog(config_get_string_value(config, "LOG_KERNEL"), "PLP");

	if (logPlp == NULL ) {
		printf("Error al crear el log PLP");
		return EXIT_FAILURE;
	}
	log_debug(logPlp, "Inicia el proceso PLP..");

	socketUMV = crearConexionConUMVYConectar(); //Socket para conectar con la UMV
	if (socketUMV == -1) {
		log_error(logPlp, "Error al conectar con la UMV");
		exit(-1);
	}
	log_debug(logPlp, "Socket conectado a la UMV correctamente");

	if (pthread_create(&threadSocket, NULL, comenzarSocketServer,
			(void*) socketUMV) != 0) {
		printf("No se pudo crear el thread para el Socket Serv - PLP");
	}

	if (pthread_create(&threadExit, NULL, vaciarColaExit, (void*) socketUMV)
			!= 0) {
		printf("No se pudo crear el thread para el Socket Serv - PLP");
	}

	if (pthread_create(&threadMensajeProg, NULL, mensajesPrograma, NULL )
			!= 0) {
		printf("No se pudo crear el thread para el Socket Serv - PLP");
	}

	while (1) {
		sem_wait(&semNuevoProg);/*Este 1er semaforo espera que se cree un nuevo pcb y que mande signal para recien
		 poder enviarlo a la cola de Ready*/
		sem_wait(&semMultiProg);/*Semaforo inicializado con el valor de Multiprogramacion*/
		log_debug(logPlp,
				"Se procede a pasar un pcb en estado New a la cola Ready, segun el grado de MP lo permita");
		list_sort(colaNew, (void*) _menorPeso);
		t_pcb *aux = list_remove(colaNew, 0);
		log_debug(logPlp, "Proceso %u pasa a la cola READY", aux->pcb_id);
		queue_push(colaReady, aux);
		sem_post(&semNuevoPcbColaReady);
	}

	pthread_join(threadSocket, NULL );
	pthread_join(threadExit, NULL );
	config_destroy(config);
	log_destroy(logPlp);

	return 0;
}
