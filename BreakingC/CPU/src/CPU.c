/*
 ============================================================================
 Name        : CPU.c
 Author      : Franco Bustos
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "CPU.h"

void CrearLog();
t_log* Log;
t_config* config;
t_dictionary* dic_variables;
char* etiquetas;
int socket_PCP;
int socket_UMV;
t_pcb* pcb;
char operacion;
bool cerrarCPU;
bool etiq_creadas;

// Funcion que crea/abre el archivo log de la cpu
void CrearLog() {
	Log = log_create(config_get_string_value(config, "LOG"), "CPU", 1,
			LOG_LEVEL_TRACE);
}

void sig_handler(int signo) {
	if (signo == SIGUSR1)
	{
		char * buffer = "Se ha recibido la senial SIGUSR1";
		log_info(Log, "Se ha recibido la senial SIGUSR1");
		if (operacion == NOTHING)
		{
			finalizarConexion(socket_PCP);
			return;
		}
		cerrarCPU = true;
		enviarDatos(socket_PCP, &buffer, strlen(buffer) + 1, SIGUSR);
		log_debug(Log, "Se termina de ejecutar la rafaga actual y luego se cierra esta CPU");
	}
}

int main(void) {
	char* sentencia;
	int i = 0;
	double quantum_ret;
	t_EstructuraInicial* est_quantum;
	config = config_create(getenv("CONFIG"));
	CrearLog();

	log_info(Log, "----------------Comienza el proceso CPU----------------");

	/*
	 * Manejo de la interrupcion SIGUSR1
	 */
	signal(SIGUSR1, sig_handler);

	log_info(Log, "Se abre conexion con el Kernel");
	if ((socket_PCP = ConectarPCP()) < 0) {
		log_error(Log, "Ha ocurrido un error durante la conexion con el PCP");
		return EXIT_FAILURE;
	}

	log_info(Log, "Se abre conexion con la UMV");
	if ((socket_UMV = ConectarUMV()) < 0) {
		log_error(Log, "Ha ocurrido un error durante la conexion con la UMV");
		return EXIT_FAILURE;
	}

	HandshakeUMV();

	log_info(Log, "Se recibe el Quantum");
	if ((est_quantum = RecibirQuantum()) < 0) {
		log_error(Log, "Error al recibir el Quantum");
		return EXIT_FAILURE;
	}
	quantum_ret = (est_quantum->RetardoQuantum) / 1000;

	log_info(Log, "Comienza la ejecucion de las sentencias");
	while (1) {
		operacion = NOTHING;

		log_info(Log, "Se recibe un PCB");
		if (RecibirPCB(&pcb) < 0) {
			log_error(Log, "Error al recibir el PCB");
			break;
		}

		operacion = SUCCESS;
		CambiarProcesoActivo();

		log_info(Log, "Se crea el Diccionario de Variables");
		//Funcion que arma el diccionario de variables a partir del pcb
		RecrearDiccionario(false);
		if (operacion == ERROR)
			break;

		log_info(Log, "Se obtiene el Array de Etiquetas");
		//Funcion que obtiene el diccionario de etiquetas
		etiq_creadas = ObtenerDiccionarioEtiquetas();
		if (operacion == ERROR)
			break;

		//Se ejecutan quantum instrucciones del programa
		for (i = 0; i < (est_quantum->Quantum); i++) {
			operacion = QUANTUM;

			pcb->PC++;
			log_debug(Log, "Se incrementa el valor del PC (Program Counter), que ahora vale: %d",
					pcb->PC);

			//Obtener proxima sentencia a ejecutar Indice de Codigo
			log_info(Log, "Se obtiene la proxima sentencia a ejecutar");
			if ((sentencia = ObtenerSentencia(pcb)) == NULL )
				break;
			log_debug(Log, "La sentencia es: %s", sentencia);

			//Parsear sentencia y Ejecutar operaciones requeridas
			log_debug(Log,
					"Se ejecuta el parser y las primitivas correspondientes");
			analizadorLinea(strdup(sentencia), &functions, &kernel_functions);

			log_debug(Log, "Se ejecuta un retardo de %d milisegundos",
					est_quantum->RetardoQuantum);
			sleep(quantum_ret);

			free(sentencia);
			if (operacion != QUANTUM)
				break;
		}

		CambioContexto();
		if (cerrarCPU)
			break;
	}
	log_debug(Log, "Cerrando CPU");

	free(est_quantum);
	if (operacion != NOTHING)
		finalizarConexion(socket_PCP);
	finalizarConexion(socket_UMV);
	log_info(Log, "CPU cerrada");
	log_destroy(Log);
	config_destroy(config);

	return EXIT_SUCCESS;
}
