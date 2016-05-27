#include "funcionesPLP.h"

extern t_config* config;
extern uint32_t nroProg;
extern sem_t semProgExit;
extern sem_t semMensajeImpresion;
extern t_queue* colaExit;
pthread_mutex_t mutex_exit;

uint32_t calcularPeso(const t_metadata_program *metaData){

	uint32_t peso;
	peso=(5*metaData->cantidad_de_etiquetas)+(3*metaData->cantidad_de_funciones)+metaData->instrucciones_size;
	return peso;

}


t_pcb* crearPCB(char* programa,int socket){

	log_debug(logPlp,"Procedo a crear el pcb del programa recibido");
	t_pcb* pcb;
	pcb=malloc(sizeof(t_pcb));
	t_metadata_program* datos;
	uint32_t base=0;
	int rtaOp=0;
	char* rtaEnvio;
	char* baseSegCod;
	char* baseSegStack;
	char* baseSegIndEtiq;
	char* baseSegIndCod;/*Variables en donde voy a guardar la base
	devuelta por la funciona de la UMV*/

	log_debug(logPlp,"Obtengo la metadata utilizando el preprocesador del parser");
	datos=metadata_desde_literal(programa);

	log_debug(logPlp,"Hacemos handshake con la UMV, avisando que somos el Kernel");
	t_handshake handshake;
	handshake.tipo=0;
	enviarOperacion(HANDSHAKE,&handshake,socket);/*Le avisamos a la UMV que el que va a enviar la operacion es el Kernel*/

	pcb->pcb_id=++nroProg;//nroProg variable global definida en el Kernel, aumenta en 1 su valor cada vez que entra un programa
	log_info(logPlp,"Enviamos el 1er pedido a la UMV, para crear el segmento de codigo literal");
	t_crear_segmento crearSegmento;
	crearSegmento.programID=pcb->pcb_id;
	crearSegmento.tamanio=strlen(programa)+1;


	baseSegCod=malloc(sizeof(uint32_t));
	baseSegCod=enviarOperacion(CREAR_SEGMENTO,&crearSegmento,socket);
	memcpy(&rtaOp,baseSegCod,sizeof(int));
		if(rtaOp < 0){
			/*Suponemos que en el caso de que no pueda crear un segmento, va a devolver -1*/
			log_error(logPlp,"No se pudo crear el segmento de codigo literal para el Programa %i",pcb->pcb_id);
			programa[0]=-1;// Indicamos con -1 para saber que el error fue por Memory Overload
			free(pcb);
			free(baseSegCod);
			free(datos);
			return NULL;
		}
	log_debug(logPlp,"Segmento de codigo literal creado!");

		log_info(logPlp,"2do pedido, segmento de stack");
		/*Obtengo el valor del stack por archivo de configuracion*/
		uint32_t tamanioStack=config_get_int_value(config,"STACK");
		crearSegmento.tamanio=tamanioStack;
		baseSegStack=malloc(sizeof(uint32_t));
		baseSegStack=enviarOperacion(CREAR_SEGMENTO,&crearSegmento,socket);
		memcpy(&rtaOp,baseSegStack,sizeof(int));
		if(rtaOp < 0){
			log_error(logPlp,"No se pudo crear el segmento de stack para el Programa %i",pcb->pcb_id);
			programa[0]=-1;
			free(baseSegCod);
			free(baseSegStack);
			free(datos);
			queue_push(colaExit,pcb);
			sem_post(&semProgExit);
			return NULL;
		}
	log_debug(logPlp,"Segmento de Stack creado!");

		log_info(logPlp,"3er pedido, segmento de indice de codigo");
		crearSegmento.tamanio=(datos->instrucciones_size)*8;
		baseSegIndCod= malloc(sizeof(uint32_t));
		baseSegIndCod=enviarOperacion(CREAR_SEGMENTO,&crearSegmento,socket);
		memcpy(&rtaOp,baseSegIndCod,sizeof(int));
		if(rtaOp < 0){
			log_error(logPlp,"No se pudo crear el segmento de codigo literal para el Programa %i",pcb->pcb_id);
			programa[0]=-1;
			free(baseSegCod);
			free(baseSegStack);
			free(baseSegIndCod);
			free(datos);
			queue_push(colaExit,pcb);
			sem_post(&semProgExit);
			return NULL;
		}
	log_debug(logPlp,"Segmento de indice de codigo creado!");

	if(datos->cantidad_de_etiquetas > 0 || datos->cantidad_de_funciones > 0){
		log_info(logPlp,"Ultimo pedido, segmento de indice de etiquetas y funciones");
		crearSegmento.tamanio=datos->etiquetas_size;
		baseSegIndEtiq=malloc(sizeof(uint32_t));
		baseSegIndEtiq=enviarOperacion(CREAR_SEGMENTO,&crearSegmento,socket);
		memcpy(&rtaOp,baseSegIndEtiq,sizeof(int));
		if(rtaOp < 0){
			log_error(logPlp,"No se pudo crear el segmento etiquetas y funciones para el Programa %i",pcb->pcb_id);
			programa[0]=-1;
			free(baseSegCod);
			free(baseSegStack);
			free(baseSegIndCod);
			free(baseSegIndEtiq);
			free(datos);
			queue_push(colaExit,pcb);
			sem_post(&semProgExit);
			return NULL;
		}
		log_debug(logPlp,"Segmento de indice de etiquetas y funciones creado!");
	}

	log_info(logPlp,"En este punto ya creamos los distintos segmentos requeridos para el programa, procedemos a"
			" escribir sobre ellos");

	log_debug(logPlp,"Enviamos a la UMV el aviso de cambio de Proceso");
	t_cambio_proc_activo procActivo;
	procActivo.programID=pcb->pcb_id;
	enviarOperacion(CAMBIO_PROC_ACTIVO,&procActivo,socket);/*Avisamos a la UMV que proceso es el que se va a escribir*/

	t_enviarBytes envioBytes;
	log_debug(logPlp,"Escribimos sobre el segmento de codigo literal");
	memcpy(&base,baseSegCod,sizeof(uint32_t));
	envioBytes.base=base;
	envioBytes.offset=0;
	envioBytes.tamanio=strlen(programa)+1;
	envioBytes.buffer=malloc(strlen(programa)+1);
	strcpy(envioBytes.buffer,programa);
		rtaEnvio=enviarOperacion(ENVIAR_BYTES,&envioBytes,socket);
		memcpy(&rtaOp,rtaEnvio,sizeof(int));
		if(rtaOp < 0){
			log_error(logPlp,"Error al tratar de escribir sobre el segmento de codigo");
			programa[0]=-2;
			free(baseSegCod);
			free(baseSegStack);
			free(baseSegIndCod);
			free(baseSegIndEtiq);
			free(datos);
			free(envioBytes.buffer);
			free(rtaEnvio);
			queue_push(colaExit,pcb);
			sem_post(&semProgExit);
			return NULL;
		}
	log_debug(logPlp,"Se ha escrito correctamente sobre el segmento de codigo");
	free(envioBytes.buffer);
	free(rtaEnvio);

	log_debug(logPlp,"2da Escritura, sobre el segmento de indice de codigo");
	memcpy(&base,baseSegIndCod,sizeof(uint32_t));
	envioBytes.base=base;
	envioBytes.offset=0;
	envioBytes.tamanio=(datos->instrucciones_size)*8;
	envioBytes.buffer=serializadoIndiceDeCodigo(datos->instrucciones_size,datos->instrucciones_serializado);
	/*La funcion toma el mapa de instrucciones, lo serializa y lo copia en el buffer*/
		rtaEnvio=enviarOperacion(ENVIAR_BYTES,&envioBytes,socket);
		memcpy(&rtaOp,rtaEnvio,sizeof(int));
		if(rtaOp < 0){
			log_error(logPlp,"Error al tratar de escribir sobre el segmento de indice de codigo");
			programa[0]=-2;
			free(baseSegCod);
			free(baseSegStack);
			free(baseSegIndCod);
			free(baseSegIndEtiq);
			free(datos);
			free(envioBytes.buffer);
			free(rtaEnvio);
			queue_push(colaExit,pcb);
			sem_post(&semProgExit);
			return NULL;
		}
	log_debug(logPlp,"Se ha escrito correctamente sobre el segmento de indice de codigo");
	free(envioBytes.buffer);
	free(rtaEnvio);

	if(datos->cantidad_de_etiquetas > 0 || datos->cantidad_de_funciones > 0){
	log_debug(logPlp,"3ra y ultimo escritura, sobre el indice de etiquetas y funciones");
	memcpy(&base,baseSegIndEtiq,sizeof(uint32_t));
	envioBytes.base=base;
	envioBytes.offset=0;
	envioBytes.tamanio=(datos->etiquetas_size);
	envioBytes.buffer=malloc(datos->etiquetas_size);
	memcpy(envioBytes.buffer,datos->etiquetas,datos->etiquetas_size);
		rtaEnvio=enviarOperacion(ENVIAR_BYTES,&envioBytes,socket);
		memcpy(&rtaOp,rtaEnvio,sizeof(int));
		if(rtaOp < 0){
			log_error(logPlp,"Error al tratar de escribir sobre el segmento de indice de etiq y funciones");
			programa[0]=-2;
			free(baseSegCod);
			free(baseSegStack);
			free(baseSegIndCod);
			free(baseSegIndEtiq);
			free(datos);
			free(envioBytes.buffer);
			free(rtaEnvio);
			queue_push(colaExit,pcb);
			sem_post(&semProgExit);
			return NULL;
		}
	log_debug(logPlp,"Se ha escrito correctamente sobre el segmento de indice de etiquetas y funciones");
	free(envioBytes.buffer);
	free(rtaEnvio);
	}

	log_debug(logPlp,"Rellenamos la estructura del pcb con las direcciones devueltas por la UMV y completamos"
			" la creacion del PCB %d",pcb->pcb_id);
	pcb->PC=datos->instruccion_inicio;
	memcpy(&base,baseSegStack,sizeof(uint32_t));
	pcb->cursor_stack=base;
	pcb->seg_stack=base;
	memcpy(&base,baseSegIndCod,sizeof(uint32_t));
	pcb->index_codigo=base;
	if(datos->cantidad_de_etiquetas > 0 || datos->cantidad_de_funciones > 0){
		memcpy(&base,baseSegIndEtiq,sizeof(uint32_t));
		pcb->index_etiq=base;
	}else{
		pcb->index_etiq=0;
	}
	pcb->peso=calcularPeso(datos);
	memcpy(&base,baseSegCod,sizeof(uint32_t));
	pcb->seg_codigo=base;
	pcb->tamanio_index_etiq=datos->etiquetas_size;
	pcb->tamanio_contexto=0;

	free(baseSegCod);
	free(baseSegIndCod);
	if(datos->cantidad_de_etiquetas>0){free(baseSegIndEtiq);}
	free(baseSegStack);
	free(datos);

	return pcb;
}

int crearConexionConUMVYConectar(void) {

	log_debug(logPlp,"Creamos el socket con el que vamos a conectar con la UMV");

	int puerto;
	int sockfd;
	char* ip;
	struct sockaddr_in socketServ;
	/*  Obtengo los datos de direccion y puerto de la UMV y lo guarda en serverInfo.*/
	puerto=config_get_int_value(config,"PUERTO_UMV");
	ip=config_get_string_value(config,"IP_UMV");

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1){
		log_error(logPlp,"Error al crear el socket para la conexion con la UMV");
		exit(1);
	}

	/*Cargamos la info del servidor UMV*/
	socketServ.sin_family=AF_INET;
	socketServ.sin_port=htons(puerto);
	socketServ.sin_addr.s_addr=inet_addr(ip);
	memset(&(socketServ.sin_zero),'\0',8);

	// Ahora me Conecto
	if(connect(sockfd,(struct sockaddr *)&socketServ,sizeof(struct sockaddr_in))==-1){
		log_error(logPlp,"Error en el connect con la UMV");
		exit(-1);
	}
	log_debug(logPlp,"Conectado al servidor UMV");

	free(ip);
	return sockfd;
}

char* serializadoIndiceDeCodigo(t_size cantInstruc,t_intructions* indiceCodigo){

	int offset=0,i=0;
	int tmp_size=sizeof(uint32_t);
	char* buffer=malloc(cantInstruc*8);

	for(i=0;i<cantInstruc;++i){
		memcpy(buffer+offset,&(indiceCodigo[i].start),tmp_size);
		offset+=tmp_size;
		memcpy(buffer+offset,&(indiceCodigo[i].offset),tmp_size);
		offset+=tmp_size;
	}

	return buffer;
}

bool _menorPeso(t_pcb *pcb1, t_pcb *pcb2){
	return pcb1->peso < pcb2->peso;
}

void* vaciarColaExit(void* socket){
	int socketUMV=(int)socket;

	while(1){
		sem_wait(&semProgExit);/*Esperamos a que se envie algun PCB a la cola exit*/
		t_pcb *aux=queue_pop(colaExit);
		pthread_mutex_lock(&mutex_exit);
		if((procDestruirSeg(aux->pcb_id,socketUMV)) < 0){
			log_error(logPlp,"Error al borrar los segmentos del pcb con pid: %d",aux->pcb_id);
		}
		else{
			log_info(logPlp,"Borrados los segmentos del programa:%d",aux->pcb_id);
		}
		free(aux);
	}
	return 0;
}

int procDestruirSeg(int32_t pcbId, int socket){
	t_destruir_segmento destruirSegmento;
	destruirSegmento.programID=pcbId;
	if((enviarOperacion(DESTRUIR_SEGMENTO,&destruirSegmento,socket))<0){
		return(-1);
	}
	return 0;
}

void* mensajesPrograma(void){

	while(1){
		sem_wait(&semMensajeImpresion);
		t_imprimibles* msjCliente;//=malloc(sizeof(t_imprimibles));
		msjCliente=queue_pop(colaImprimibles);
		t_socket_pid *aux;
		log_info(logPlp,"Ingreso un nuevo mensaje que sera impreso por el Programa %u",msjCliente->PCB_ID);

		bool _encuentraCliente(t_socket_pid* socket_pid){
			return socket_pid->pid==msjCliente->PCB_ID;
		}

		aux=list_find(listaClientesProgramas,(void*) _encuentraCliente);
		if (aux==NULL) {
			free(msjCliente);
			log_error(logPlp,"No se llego a imprimir porque el programa habia sido cerrado");
			return 0;
		}
		switch(msjCliente->tipoDeValor){
			case IMPRIMIR_TEXTO:
				if((enviarDatos(aux->socket,&msjCliente->valor,strlen(msjCliente->valor)+1,IMPRIMIR_TEXTO)) < 0){
					log_error(logPlp,"Error al enviar un mensaje a imprimir:TEXTO por el Programa: %u",aux->pid);
				}else{
					log_debug(logPlp,"Mensaje de tipo:TEXTO enviado al Programa: %u con éxito!",aux->pid);
				}
				break;
			case IMPRIMIR_VALOR:
				if((enviarDatos(aux->socket,&msjCliente->valor,sizeof(uint32_t),IMPRIMIR_VALOR)) < 0){
					log_error(logPlp,"Error al enviar un mensaje a imprimir de tipo:VALOR por el Programa: %u",aux->pid);
				}else{
					log_debug(logPlp,"Mensaje de tipo:VALOR enviado al Programa: %u con éxito!",aux->pid);
				}
				break;
			case EXIT:
				aux->terminado=true;
				if((enviarDatos(aux->socket,&msjCliente->valor,strlen(msjCliente->valor)+1,EXIT)) < 0){
					log_error(logPlp,"Error al enviar un mensaje a imprimir de tipo:EXIT por el Programa: %u",aux->pid);
				}else{
					log_debug(logPlp,"Mensaje de tipo:EXIT enviado al Programa: %u con éxito!",aux->pid);
				}

				pthread_mutex_unlock(&mutex_exit);
				break;
			case ERROR:
				aux->terminado=true;
				if((enviarDatos(aux->socket,&msjCliente->valor,strlen(msjCliente->valor)+1,ERROR)) < 0){
					log_error(logPlp,"Error al enviar un mensaje a imprimir de tipo:ERROR por el Programa: %u",aux->pid);
				}else{
					log_debug(logPlp,"Mensaje de tipo:ERROR enviado al Programa: %u con éxito!",aux->pid);
				}
				pthread_mutex_unlock(&mutex_exit);
				break;
			default:
				log_error(logPlp,"El tipo de mensaje ingresado no es soportado por la interface");
				break;
		}
		free(msjCliente);
	}

	return 0;
}
