#include "kernel.h"

t_log *log_kernel;
t_list* colaNew;
t_queue* colaReady;
t_queue* colaExit;
t_queue* colaImprimibles;
uint32_t nroProg=0; //id programa
int multiProg;
sem_t semMultiProg;
sem_t semNuevoProg;
sem_t semProgExit;
sem_t semNuevoPcbColaReady;
sem_t semProgramaFinaliza;
sem_t semMensajeImpresion;
int32_t pcbAFinalizar; // pcb q vamos a cerrar porque el programa cerro mal


int main(void){

	// ######################## Inicializaciones ######################## //
	t_config *config=config_create(getenv("CONFIG_KERNEL"));
	log_kernel=creacionLog(config_get_string_value(config,"LOG_KERNEL"),"Kernel");
	log_debug(log_kernel,"Creado log del Kernel");
	pthread_t tPCP;							// Hilo dedicado al PCP
	pthread_t tPLP;							// Hilo dedicado al PLP
	colaNew=list_create();					// Lista para procesos en estado NEW
	colaReady=queue_create();				// Cola para procesos en estado READY
	colaExit=queue_create();				// Cola para procesos en estado EXIT
	colaImprimibles=queue_create();			// Cola para valores a imprimir por el PLP
//	listaBloqueados= list_create();			// Lista para procesos BLOCKED por semaforo

	multiProg=config_get_int_value(config,"MULTIPROGRAMACION");
	log_debug(log_kernel,"Seteo el grado de multiprogramacion permitido");

	sem_init(&semMultiProg,1,multiProg); // Parametro 1 es para que sea compartido
	sem_init(&semNuevoProg,1,0);		// Para sacar de la cola New solo cuando se manda señal de que se creó uno
	sem_init(&semProgExit,1,0);			// Misma funcion que semNuevoProg pero para los prog en Exit
	sem_init(&semNuevoPcbColaReady,1,0);// Semaforo que avisa cuando la cola de ready le agregan un PCB
	sem_init(&semMensajeImpresion,1,0);// Semaforo para revisar los mensajes a mandar a los programas para imprimir
	sem_init(&semProgramaFinaliza,1,0);


	// ######################## Fin inicializaciones ######################## //
	//pruebaCreacionPCBS();

	if(pthread_create(&tPCP,NULL,(void*)comienzaPCP,NULL)!=0){
		printf("No se pudo crear el thread para el PCP");
	}

	if(pthread_create(&tPLP,NULL,(void*)comienzaPLP,NULL)!=0){
		printf("No se pudo crear el thread para el PLP");
	}

	// Dsp ver de ponerlos en un if
	pthread_join(tPCP,NULL);
	pthread_join(tPLP,NULL);

	return 0;
}
