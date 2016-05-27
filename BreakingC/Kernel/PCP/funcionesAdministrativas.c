/*
 * funcionesAdministrativas.c
 *
 *  Created on: 13/07/2014
 *      Author: utnso
 */
#include "pcp.h"
t_list * listaClientes, *listaVarCompartidas, *listaSemaforos, *listaIO, *listaBloqueados;
t_log* log_pcp;
pthread_mutex_t mutex;
extern int cpuAcerrar;
extern int32_t pcbAFinalizar;

int cuantosClientesSinPCB() {
	int cantClientSinPCB;		// Cantidad de clientes(CPUS) que no tienen PCB
	bool _sinPCB(t_clienteCPU * unCliente) {
		return unCliente->fueAsignado == false;
	}
	cantClientSinPCB = list_size(list_filter(listaClientes, (void*) _sinPCB));
	return cantClientSinPCB;
}
void cargoVarCompartidasEnLista() {
	char ** variablesCompartidas;
	t_varCompartida *varCompartida;

	variablesCompartidas = config_get_array_value(config, "VARIABLES_COMPARTIDAS");
	int ivar = 0;
//	log_trace(log_pcp, ":: Cargo variables compartidas ::");
	while (variablesCompartidas[ivar] != NULL ) {
		varCompartida = malloc(sizeof(t_varCompartida));
		varCompartida->nombre = malloc(strlen(variablesCompartidas[ivar]) + 1);
		strcpy(varCompartida->nombre, variablesCompartidas[ivar]);
		varCompartida->valor = 0;
		list_add(listaVarCompartidas, varCompartida);
		log_trace(log_pcp, "(\"%s\", %d)", varCompartida->nombre,
				varCompartida->valor);
		ivar++;
	}/*
	 void _destroyStrings(char*str){
	 free(str);
	 }
	 string_iterate_lines(variablesCompartidas,_destroyStrings);*/
	free(variablesCompartidas);
	log_trace(log_pcp, ":: Finalizada la carga de la listaVarCompartida ::");
}
void cargoSemaforosEnLista() {
	char ** semaforos;					// Aca voy a cargar todos los semaforos
	char ** values_Semaforos;			// Valores de los semaforos
	t_semaforo *semaforo;

	semaforos = config_get_array_value(config, "SEMAFOROS");
	values_Semaforos = config_get_array_value(config, "VALOR_SEMAFOROS");
	int ivar = 0;
//	log_trace(log_pcp, ":: Cargo Semaforos ::");
	while (semaforos[ivar] != NULL ) {
		semaforo = malloc(sizeof(t_semaforo));
		semaforo->nombre = semaforos[ivar];
		semaforo->valor = atoi(values_Semaforos[ivar]);
		list_add(listaSemaforos, semaforo);
		log_trace(log_pcp, "(\"%s\", %d)", semaforo->nombre, semaforo->valor);
		ivar++;

	}
	free(semaforos);
	free(values_Semaforos);
	log_trace(log_pcp, ":: Finalizada la carga de la listaSemaforos ::");
}
void cargoIOenLista() {
	char ** idIO;					// Aca voy a cargar todos los IO
	char ** values_IO;				// Valores de los IO
	t_IO *entradaSalida;

	idIO = config_get_array_value(config, "ID_HIO");
	values_IO = config_get_array_value(config, "HIO");

	int ivar = 0;
//	log_trace(log_pcp, ":: Cargo IO ::");
	while (idIO[ivar] != NULL ) {
		entradaSalida = malloc(sizeof(t_IO));
		entradaSalida->nombre = idIO[ivar];
		entradaSalida->valor = atoi(values_IO[ivar]);
		entradaSalida->ID_IO = ivar;
		entradaSalida->colaSolicitudes = queue_create();
		sem_init(&entradaSalida->semaforo, 1, 0);
		list_add(listaIO, entradaSalida);

		log_trace(log_pcp, "(\"%s\", valor:%d, ID:%d)", entradaSalida->nombre,
				entradaSalida->valor, entradaSalida->ID_IO);
		ivar++;
	}
	free(idIO);
	free(values_IO);
	log_trace(log_pcp, ":: Finalizada la carga de la lista de IO ::");
}

void imprimoListaSemaforos() {
	log_info(log_pcp, ":: Valores actuales Semaforos ::");
	void _imprimoCadaSemaforo(t_semaforo * elem) {
		log_info(log_pcp, "(\"%s\", %d)", elem->nombre, elem->valor);
	}
	list_iterate(listaSemaforos, (void*) _imprimoCadaSemaforo);
	log_info(log_pcp, ":::: :::: :::: :::: :::: :::: :::: :::: ::::");
}
void imprimoListaIO() {
	log_info(log_pcp, ":: Valores actuales Dispositivos IO ::");
	void _imprimoCadaIO(t_IO * elem) {
		log_info(log_pcp, "(\"%s\", valor:%d, ID:%d )", elem->nombre,
				elem->valor, elem->ID_IO);
	}
	list_iterate(listaIO, (void*) _imprimoCadaIO);
	log_info(log_pcp, ":::: :::: :::: :::: :::: :::: :::: :::: ::::");
}
void imprimoListaVarCompartida() {
	log_info(log_pcp, ":::: Valores actuales variables compartidas ::::");
	void _imprimoCadaVariable(t_varCompartida * elem) {
		log_info(log_pcp, "(\"%s\", %d)", elem->nombre, elem->valor);
	}
	list_iterate(listaVarCompartidas, (void*) _imprimoCadaVariable);
	log_info(log_pcp, ":::: :::: :::: :::: :::: :::: :::: :::: ::::");
}
void imprimoListaClientesCPU() {
	void _imprimoCadaClienteCPU(t_clienteCPU * elem) {
		if (elem->fueAsignado == true) {
			log_info(log_pcp, "## Cliente CPU:%d, PCB asignado:%d, socket:%d",
					elem->ID, elem->pcbAsignado->pcb_id, elem->socket);
		} else {
			log_info(log_pcp, "## Cliente CPU:%d, PCB asignado:%d, socket:%d",
					elem->ID, elem->fueAsignado, elem->socket);
		}
	}
	list_iterate(listaClientes, (void*) _imprimoCadaClienteCPU);
}
void imprimoListaBloqueados() {
	void _imprimoCadaPcbBlocked(t_pcbBlockedSemaforo * elem) {
		log_info(log_pcp, "## PCB_ID:%d, semaforo:%s", elem->pcb->pcb_id,
				elem->nombreSemaforo);
	}
	list_iterate(listaBloqueados, (void*) _imprimoCadaPcbBlocked);
}
void imprimoInformacionGeneral() {

	log_info(log_pcp, "#### #### #### #### #### ####");
	log_info(log_pcp, "#### Cant clientesCPU ociosos:%d",
			cuantosClientesSinPCB());
	log_info(log_pcp, "#### #### #### #### #### ####");
	log_info(log_pcp, "#### Cant clientesCPU:%d ####",
			list_size(listaClientes));
	imprimoListaClientesCPU();
	log_info(log_pcp, "#### #### #### #### #### ####");

	log_info(log_pcp, "#### elementos en colaReady:%d ####",
			queue_size(colaReady));
	recorrerUnaColaDePcbs(colaReady);
	log_info(log_pcp, "#### #### #### #### #### ####");

	log_info(log_pcp, "#### elementos en colaExit:%d ####",
			queue_size(colaExit));
	recorrerUnaColaDePcbs(colaExit);
	log_info(log_pcp, "#### #### #### #### #### ####");

	log_info(log_pcp, "#### elementos en lista Bloqueados:%d ####",
			list_size(listaBloqueados));
	imprimoListaBloqueados();

	log_info(log_pcp, "#### CPU a finalizar=%d , PCB a finalizar=%d####",cpuAcerrar, pcbAFinalizar);
	log_info(log_pcp, "#### Valores semaforos ####");
	log_info(log_pcp, "Semaforo clienteOcioso:%d",semClienteOcioso.__align);
	log_info(log_pcp, "Semaforo NuevoPcbColaReady:%d",semNuevoPcbColaReady.__align);
	log_info(log_pcp, "Semaforo ProgExit:%d",semProgExit.__align);
	log_info(log_pcp, "Semaforo ProgramaFinaliza:%d",semProgramaFinaliza.__align);
	log_info(log_pcp, "Semaforo MultiProg:%d",semMultiProg.__align);
	log_info(log_pcp, "Semaforo MensajeImpresio:%d",semMensajeImpresion.__align);
	log_info(log_pcp, "Semaforo NuevoProg:%d",semNuevoProg.__align);

	log_info(log_pcp, "#### #### #### #### #### ####");
}

void recorrerUnaColaDePcbs(t_queue* colaDeterminada) {
	/*
	 * Sabemos que esto esta mal conceptualmente, pero nuestro ayudante nos dijo que
	 * esta bueno hacer esto para facilitar y agilizar la correcion, asi que decidimos
	 * implementarlo
	 */
	t_queue * colaAuxiliar;
	t_pcb * unPCB;
	colaAuxiliar = queue_create();

	pthread_mutex_lock(&mutex);
	while (queue_size(colaDeterminada) > 0) {
		unPCB = queue_pop(colaDeterminada);
		log_info(log_pcp, "## PCB_ID:%d", unPCB->pcb_id);
		queue_push(colaAuxiliar, unPCB);
	}
	while (queue_size(colaAuxiliar) > 0) {
		unPCB = queue_pop(colaAuxiliar);
		queue_push(colaDeterminada, unPCB);
	}
	pthread_mutex_unlock(&mutex);

	queue_destroy(colaAuxiliar);
}
