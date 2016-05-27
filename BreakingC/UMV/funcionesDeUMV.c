/*
 * funcionesDeUMV.c
 *
 *  Created on: 11/05/2014
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include "estructuras.h"
#include "UMV_Main.h"
#include <commons/log.h>
#include <pthread.h>
#include <string.h>
#include "funcionesDeUMV.h"
extern pthread_mutex_t escribirListaUMV;
extern pthread_mutex_t semaforoTipoProceso;

typedef enum {
	WORSTFIT, FIRSTFIT

} algortimo;

extern t_list *UMV;
extern t_log* ptrLog;
//extern int pidActivo;
extern int tiempoRetardo;
//extern int tipoProceso;
extern int tipoAlgoritmo;

t_segmento* crearSegmentoLocal(int, int, t_segmento*);
int modificarSegmentoLocal(t_segmento*, int, int, int);
int cambiarProcesoActivo(t_cambio_proc_activo*, int*);
t_segmento* fragmentar(int, t_segmento*, t_segmento*);
//void compactar();

char* solicitarBytes(t_solicitarBytes *solicitarBytes, int* procesoActivo) {
	//char * buffer = malloc(solicitarBytes->tamanio );
	/*	char * buffer = malloc(100);
	 if (solicitarBytes->offset >=solicitarBytes->tamanio) {
	 log_debug(ptrLog,"Direccion no valida segun el libro, pag 327 primeras lineas");
	 return "-1";

	 }else{*/
	int respuestaEnCasoDeError = -1;

	bool _mismoPid(t_segmento* segmento) {
		return segmento->pid == *procesoActivo;
	}
	bool _buscoBase(t_segmento * segmento) {
		return segmento->posInicial == solicitarBytes->base
				&& segmento->pid == *procesoActivo
				&& segmento->segmentoReservado == 1;
	}
	pthread_mutex_lock(&escribirListaUMV);
	//t_list *segmentosDelPid = list_create();
	//segmentosDelPid= list_filter(UMV, (void*) _mismoPid);
	t_segmento* segmentoEncontrado = list_find(UMV, (void*) _buscoBase);
	/*AGREGADO VALIDACION POR SI NO ENCUENTRA SEGMENTO*/
	if (segmentoEncontrado == NULL ) {
		char* buffer = malloc(sizeof(int));
		memcpy(buffer, &respuestaEnCasoDeError, sizeof(int));
		sleep(tiempoRetardo);
		log_error(ptrLog, "No se econtro un segmento con los datos pedidos");
		pthread_mutex_unlock(&escribirListaUMV);
		return buffer;
	}
	if (solicitarBytes->offset > segmentoEncontrado->size) {
		char* buffer = malloc(sizeof(int));
		memcpy(buffer, &respuestaEnCasoDeError, sizeof(int));
		sleep(tiempoRetardo);
		log_error(ptrLog,
				"El offset que se pide es mayor al tamanio del segmento");
		pthread_mutex_unlock(&escribirListaUMV);
		return buffer;

	}
	if ((solicitarBytes->offset + solicitarBytes->tamanio) > segmentoEncontrado->size) {
			char* buffer = malloc(sizeof(int));
			memcpy(buffer, &respuestaEnCasoDeError, sizeof(int));
			sleep(tiempoRetardo);
			log_error(ptrLog,
					"El offset que se pide es mayor al tamanio del segmento");
			pthread_mutex_unlock(&escribirListaUMV);
			return buffer;

		}
	if (solicitarBytes->tamanio > segmentoEncontrado->size) {
		char* buffer = malloc(sizeof(int));
		memcpy(buffer, &respuestaEnCasoDeError, sizeof(int));
		sleep(tiempoRetardo);
		log_error(ptrLog,
				"El tamanio que se pide es mayor al tamanio del segmento");
		pthread_mutex_unlock(&escribirListaUMV);
		return buffer;

	}
	if (solicitarBytes->tamanio <= 0) {
		char* buffer = malloc(sizeof(int));
		memcpy(buffer, &respuestaEnCasoDeError, sizeof(int));
		sleep(tiempoRetardo);
		log_error(ptrLog, "El tamanio que se pide es menor a 0");
		pthread_mutex_unlock(&escribirListaUMV);
		return buffer;

	}

	/*AGREGADO VALIDACION POR SI NO ENCUENTRA SEGMENTO*/
	log_debug(ptrLog, "##### Respecto al segmento ####");
	log_debug(ptrLog, "Pid:%d", (segmentoEncontrado->pid));
	log_debug(ptrLog, "Base: %d", segmentoEncontrado->posInicial);
	log_debug(ptrLog, "PosMemoria:%d", segmentoEncontrado->posMemoria);
	log_debug(ptrLog, "segmentoReservado: %d",
			segmentoEncontrado->segmentoReservado);
	log_debug(ptrLog, "Size de Segmento: %d", segmentoEncontrado->size);
	log_debug(ptrLog, "##### Respecto a la solicitud ####");
	log_debug(ptrLog, "Base: %d", solicitarBytes->base);
	log_debug(ptrLog, "Offset: %d", solicitarBytes->offset);
	log_debug(ptrLog, "Tamanio: %d", solicitarBytes->tamanio);

	void* c;
	c = (void*)((segmentoEncontrado->posMemoria) + (solicitarBytes->offset));
	char* buffer = calloc(solicitarBytes->tamanio, (sizeof(char)));
	memcpy(buffer, (char*) c, solicitarBytes->tamanio);
	//printf("\n\nProbando, datos en la direc de memoria en buffer:%s\n\n",
	//buffer);
	sleep(tiempoRetardo);
	pthread_mutex_unlock(&escribirListaUMV);
	return buffer;

	//}
}

char* enviarBytes(t_enviarBytes *enviarBytes, int procesoActivo) {
	/*if (enviarBytes->offset >=enviarBytes->tamanio) {
	 log_debug(ptrLog,"Direccion no valida segun el libro, pag 327 primeras lineas");
	 return "-1";*/

//	}else{
	sleep(tiempoRetardo);
	pthread_mutex_lock(&escribirListaUMV);
	char* respuesta = malloc(sizeof(int));
	int resultado = 0;
	bool _mismoPid(t_segmento* segmento) {
		return segmento->pid == procesoActivo;
	}
	bool _buscoBase(t_segmento * segmento) {
		return segmento->posInicial == enviarBytes->base
				&& segmento->pid == procesoActivo
				&& segmento->segmentoReservado == 1;
	}
	t_segmento* segmentoEncontrado = list_find(UMV, (void*) _buscoBase);

	/*AGREGADO VALIDACION POR SI NO ENCUENTRA SEGMENTO*/
	if (segmentoEncontrado == NULL ) {	//No encontro el segmento
		resultado = -1;
		memcpy(respuesta, &resultado, sizeof(int));
		pthread_mutex_unlock(&escribirListaUMV);
		return respuesta;
	}

	/*VALIDAR si esto esta bien SACARLOOOO
	if (strlen(enviarBytes->buffer) > enviarBytes->tamanio) {
		//Se quiere mandar mas de lo que declara
		resultado = -1;
		memcpy(respuesta, &resultado, sizeof(int));
		pthread_mutex_unlock(&escribirListaUMV);
		return respuesta;
	}*/

	if (enviarBytes->tamanio + enviarBytes->offset > segmentoEncontrado->size) {
		//Se quiere mandar mas de lo que hya disponible
		resultado = -1;
		memcpy(respuesta, &resultado, sizeof(int));
		pthread_mutex_unlock(&escribirListaUMV);
		return respuesta;
	}

	/*AGREGADO VALIDACION POR SI NO ENCUENTRA SEGMENTO*/
	log_debug(ptrLog, "##### Respecto al segmento ####");
	log_debug(ptrLog, "Pid:%d", (segmentoEncontrado->pid));
	log_debug(ptrLog, "Base: %d", segmentoEncontrado->posInicial);
	log_debug(ptrLog, "PosMemoria:%d", segmentoEncontrado->posMemoria);
	log_debug(ptrLog, "segmentoReservado: %d",
			segmentoEncontrado->segmentoReservado);
	log_debug(ptrLog, "Size de Segmento: %d", segmentoEncontrado->size);
	log_debug(ptrLog, "##### Respecto a la solicitud ####");
	log_debug(ptrLog, "Base: %d", enviarBytes->base);
	log_debug(ptrLog, "Offset: %d", enviarBytes->offset);
	log_debug(ptrLog, "Tamanio: %d", enviarBytes->tamanio);
	//log_debug(ptrLog, "Buffer: %c", enviarBytes->buffer);
	char* c;

	c = (char*)((segmentoEncontrado->posMemoria) + (enviarBytes->offset));

	memcpy(c, enviarBytes->buffer, enviarBytes->tamanio);	//verifica aca

	memcpy(respuesta, &resultado, sizeof(int));
	pthread_mutex_unlock(&escribirListaUMV);
	return respuesta;

	//}
}

int crearSegmento(t_crear_segmento* segmentoACrear) {

	pthread_mutex_lock(&escribirListaUMV);

	t_list* ptrAux; //PTR aux de UMV

	t_segmento* nodoCero; // aux a segmento cero
	t_segmento* segmento;
	t_segmento* segmentoAux;
	t_segmento* segmentoAnterior;
	//int tipoAlgoritmo;
	int i;
	int cantElementos = 0;
	int diferenciaEntreSegmentos;
	int noHiceNada = 0;
	int yaCompacte = 0;

	nodoCero = list_get(UMV, 0);
	log_debug(ptrLog, "Inicializo crearSegmento y obtengo el segmento guia");
	for (i = 0; i < 2; ++i) {
		//me fijo si hay al menos un segmento
		//Me fijo si se inicio la lista, si no la creo
		if (((cantElementos = list_size(UMV)) == 1)
				&& (nodoCero->size >= segmentoACrear->tamanio)) {

			//CREACION DE PRIMER SEGMENTO
			log_debug(ptrLog,
					"La cantidad de elementos de la lista es 1, por lo tanto tengo que crear el primer segmento");
			segmentoAux /*= malloc(sizeof(segmentoACrear))*/;
			log_debug(ptrLog, "El tamanio del segmentos para el malloc es : %d",
					(segmentoACrear->tamanio));
			segmentoAux = crearSegmentoLocal(segmentoACrear->programID,
					segmentoACrear->tamanio, nodoCero);
			//ajusto esto porque sino me queda con un tamaño gigante
			log_debug(ptrLog,
					"Le agrego la posicion inicial al segmento a crear, tiene que ser 0 ya que es el primer segmento con datos");
			segmentoAux->posInicial = 0;
			segmentoAux->posMemoria = nodoCero->posMemoria;
			sleep(tiempoRetardo);
			pthread_mutex_unlock(&escribirListaUMV);
			return segmentoAux->posInicial;

		}

		//Si hay entro a firstFit o WF

		else {
			log_debug(ptrLog, "Si hay más de un segmento, entro en FF o WF");
			if (tipoAlgoritmo == WORSTFIT) {
				log_debug(ptrLog, "WorstFit");
				bool _estaLibreYtieneEspacio(t_segmento* segmento) {
					return ((segmento->segmentoReservado == 0)
							&& (segmento->size >= segmentoACrear->tamanio));
				}

				t_list* listaWF = list_filter(UMV,
						(void*) _estaLibreYtieneEspacio);
				log_debug(ptrLog,
						"Tengo la lista filtrada por lis segmentos libres");
				bool _ordenarPorMayor(t_segmento* segmento1,
						t_segmento* segmento2) {
					return segmento1->size > segmento2->size;
				}
				list_sort(listaWF, (void*) _ordenarPorMayor);
				log_debug(ptrLog, "Tengo la lista ordenada de mayor a menor");
				ptrAux = list_take(listaWF, 1);
				log_debug(ptrLog,
						"Tome el primer elemento, ya que es el segmento mas grande");
				//Ya tengo en segmento el puntero a modificar
				t_link_element* elementoAux;	// = malloc(sizeof(t_segmento));
				elementoAux = ptrAux->head;
				t_segmento* segmentoWF = elementoAux->data;
				//ptrAux->head->data;

				if (segmentoWF != NULL ) {

					log_debug(ptrLog, "Hubo un segmento encontrado para el WF");
					segmentoWF->pid = segmentoACrear->programID;
					segmentoWF->segmentoReservado = 1;
					log_debug(ptrLog, "Asigno el PID");
					if (segmentoWF->size > segmentoACrear->tamanio) {
						log_debug(ptrLog, "Tengo que fragmentar");
						diferenciaEntreSegmentos = segmentoWF->size
								- segmentoACrear->tamanio;
						//segmentoWF->size = segmentoACrear->tamanio;
						/*Crear nuevo segmento con lo fragmentado*/
						segmentoAux = fragmentar(diferenciaEntreSegmentos,
								nodoCero, segmentoWF);
						segmentoAux->segmentoReservado = 0;
						log_debug(ptrLog, "Fragmenté");
						noHiceNada = 0;
					} else {
						segmentoWF->size = segmentoACrear->tamanio;
						log_debug(ptrLog, "No fragmente porque entraba justo");
					}
					sleep(tiempoRetardo);
					free(listaWF);
					free(ptrAux);
					pthread_mutex_unlock(&escribirListaUMV);
					return segmentoWF->posInicial;
				} else
					noHiceNada = 1;
			}

			if (tipoAlgoritmo == FIRSTFIT) {
				log_debug(ptrLog, "FirstFit");
				bool _estaLibreYTieneEspacio(t_segmento* segmento) {
					return segmento->size >= segmentoACrear->tamanio
							&& !segmento->segmentoReservado;
				}
				//segmentoAnterior = malloc(sizeof(t_segmento));
				segmentoAnterior = list_get(UMV, list_size(UMV) - 1);
				t_segmento* segmentoFF /*= malloc(sizeof(t_segmento))*/;
				segmentoFF = list_find(UMV, (void*) _estaLibreYTieneEspacio);
				log_debug(ptrLog,
						"Me fijo los que estan libres y tienen espacio");
				if (segmentoFF != NULL ) {
					log_debug(ptrLog, "Hay un segmento libre");
					segmentoFF->pid = segmentoACrear->programID;
					segmentoFF->segmentoReservado = 1;
					if (segmentoFF->size > segmentoACrear->tamanio) {
						log_debug(ptrLog, "Hay que fragmentar");
						diferenciaEntreSegmentos = segmentoFF->size
								- segmentoACrear->tamanio;
						//segmentoFF->size = segmentoACrear->tamanio;
						/*Crear nuevo segmento con lo fragmentado*/
						segmentoAux = fragmentar(diferenciaEntreSegmentos,
								nodoCero, segmentoFF);
						segmentoAux->segmentoReservado = 0;
						log_debug(ptrLog, "Fragmenté");
						noHiceNada = 0;
					} else {
						segmentoFF->size = segmentoACrear->tamanio;
						log_debug(ptrLog, "No fragmenté");
					}
					sleep(tiempoRetardo);
					pthread_mutex_unlock(&escribirListaUMV);
					return segmentoFF->posInicial;
				}

				else
					noHiceNada = 1;
			}

			if (noHiceNada == 1) {
				log_debug(ptrLog, "No encontré segmentos libres en FF o WF.");
				log_debug(ptrLog,
						"Me fijo si hay lugar para crear segmentos nuevos.");
				if (nodoCero->size >= segmentoACrear->tamanio) {
					segmentoAux = crearSegmentoLocal(segmentoACrear->programID,
							segmentoACrear->tamanio, nodoCero);

					log_debug(ptrLog,
							"Hubo lugar y cree el segmento para el programa:%d",
							segmentoACrear->programID);
					noHiceNada = 0;
					sleep(tiempoRetardo);
					pthread_mutex_unlock(&escribirListaUMV);
					return segmentoAux->posInicial;

				} else {
					log_debug(ptrLog, "Tengo que compactar");
					if (yaCompacte == 0) {
						pthread_mutex_unlock(&escribirListaUMV);
						compactacion();
						yaCompacte = 1;
						//pthread_mutex_unlock(&escribirListaUMV);
					} else {
						log_error(ptrLog, "Ya compacte una vez, no hay lugar");
						pthread_mutex_unlock(&escribirListaUMV);
						return -1;;
					}
				}

			}
		}
	}
	//pthread_mutex_unlock(&escribirListaUMV);
}

int destruirSegmento(t_destruir_segmento* segmentoADestruir) {
	pthread_mutex_lock(&escribirListaUMV);

	bool _buscaPID(t_segmento* segmento) {
		return segmento->pid == segmentoADestruir->programID;
	}

	log_debug(ptrLog, "Dentro de DestruirSegmento funcion");
	t_list* listaDestruirSegmento = list_filter(UMV, (void*) _buscaPID);
	/*Validacion*/
	if (listaDestruirSegmento->head == NULL ) {
		log_debug(ptrLog, "No hay segmentos para destruir");
		pthread_mutex_unlock(&escribirListaUMV);
		sleep(tiempoRetardo);
		return -1;
	}
	/**/
	void _desvincularSegmentoDePID(t_segmento* segmento) {
		segmento->pid = -1;
		segmento->segmentoReservado = 0;
	}
	sleep(tiempoRetardo);
	list_iterate(listaDestruirSegmento, (void*) _desvincularSegmentoDePID);

	free(listaDestruirSegmento);
	//free(segmentoADestruir);
	pthread_mutex_unlock(&escribirListaUMV);

	return 0;

}

t_segmento* crearSegmentoLocal(int pid, int size, t_segmento* nodoCero) {

	t_segmento* segmentoAnterior;
	t_segmento* segmentoNuevo;
	log_debug(ptrLog, "Estoy en CrearSegmentoLocal");
	//segmentoAnterior = malloc(sizeof(t_segmento));
	segmentoNuevo = malloc(sizeof(t_segmento));
	log_debug(ptrLog, "Hago los mallocs en CrearSegmentoLocal");
	//printf("\n%d\n", list_size(UMV));

	segmentoAnterior = list_get(UMV, list_size(UMV) - 1);

	log_debug(ptrLog, "Guardo el segmento anterior");
	segmentoNuevo->pid = pid;
	segmentoNuevo->size = size;
	log_debug(ptrLog,
			"Guardo los valores en el segmento nuevo, PID y SIZE : %d %d", pid,
			size);
	log_debug(ptrLog,
			"Los valores SIZE y posMemoria y posInicial del segmento anterior: %d %d %d",
			segmentoAnterior->size, segmentoAnterior->posMemoria,
			segmentoAnterior->posInicial);
	segmentoNuevo->posMemoria = segmentoAnterior->size
			+ segmentoAnterior->posMemoria;
	segmentoNuevo->posInicial = segmentoAnterior->size
			+ segmentoAnterior->posInicial;
	log_debug(ptrLog,
			"Calculo posMemoria y PosInicial del segmento nuevo: %d %d",
			segmentoNuevo->posMemoria, segmentoNuevo->posInicial);
	segmentoNuevo->segmentoReservado = 1;
	//Actualizar el nodo 0
	if ((nodoCero->size > 0))
		nodoCero->size = nodoCero->size - segmentoNuevo->size;
	//Agrego el nodo nuevo
	list_add(UMV, segmentoNuevo);

	return segmentoNuevo;
}

int modificarSegmentoLocal(t_segmento* segmentoModificar, int nuevoPid,
		int nuevoSize, int operacion) {

	int diferenciaParaFragmentar = 0;
	segmentoModificar->pid = nuevoPid;
	diferenciaParaFragmentar = segmentoModificar->size - nuevoSize;
	segmentoModificar->size = nuevoSize;
	return diferenciaParaFragmentar;
}

t_segmento* fragmentar(int size, t_segmento* nodoCero,
		t_segmento* segmentoAFragmentar) {
	t_segmento* segmentoNuevo;
	t_link_element* nodo;
	nodo = UMV->head;
	segmentoNuevo = malloc(sizeof(t_segmento));
	int index = 0;

	//Averiguo la posicion del segmento a fragmentar
	while (nodo->data != segmentoAFragmentar) {
		++index;
		nodo = nodo->next;
	}
	//Antes de fragmentar, achico el segmento a fragmentar
	segmentoAFragmentar->size = segmentoAFragmentar->size - size;
	//Preparo el nuevo segmento a crear
	segmentoNuevo->pid = -1;
	segmentoNuevo->posInicial = segmentoAFragmentar->posInicial
			+ segmentoAFragmentar->size;
	segmentoNuevo->posMemoria = segmentoAFragmentar->posMemoria
			+ segmentoAFragmentar->size;
	segmentoNuevo->segmentoReservado = 0;
	segmentoNuevo->size = size;
	//Agego el segmento nuevo, luego del segmento fragmentado
	list_add_in_index(UMV, index + 1, segmentoNuevo);

	return segmentoNuevo;

}

int cambiarProcesoActivo(t_cambio_proc_activo* cambioProcesoActivo,
		int* procesoActivo) {
	sleep(tiempoRetardo);
	(*procesoActivo) = cambioProcesoActivo->programID;
	log_debug(ptrLog,"Se puso como activo el PID : %d", *procesoActivo);
	free(cambioProcesoActivo);
	return 0;

}

int handshake(t_handshake* handshake, int* tipoProceso) {
	pthread_mutex_lock(&semaforoTipoProceso);

	sleep(tiempoRetardo);
	(*tipoProceso) = handshake->tipo;
	log_debug(ptrLog,"Se conecto %d 0 = kernel, 1= CPU",*tipoProceso);
	pthread_mutex_unlock(&semaforoTipoProceso);
	free(handshake);
	return 0;
}

void compactar() {
	t_link_element* nodo;
	t_link_element* nodoSiguiente;
	t_segmento* segmento;	//segmento libre
	t_segmento* segmentoSigueinteReservado;	//segmento ocupado
	t_segmento* segmentoSiguiente;
	t_segmento* segmentoCero;
	int linea = 0;
	int diferenciaEntreSegmentos = 0;
	int cantidadDeSegmentosDeDiferencia = 0;
	int i;
	segmentoCero = list_get(UMV, 0);
	//Ya lo asigno al segundo segmento, ya que el primero es el segmento guia
	if (UMV->elements_count != 1) {
		nodo = UMV->head->next;
	} else
		goto error;
	segmento = nodo->data;
	while (nodo != NULL ) {
		libre: if (segmento->segmentoReservado == 0) {
			//tomo el siguiente
			nodoSiguiente = nodo->next;
			if (nodoSiguiente == NULL )
				goto error;
			segmentoSigueinteReservado = nodoSiguiente->data;
			//Me fijo si está ocupado, si no lo está, sigo buscando
			while (segmentoSigueinteReservado->segmentoReservado != 1) {
				nodoSiguiente = nodoSiguiente->next;
				if (nodoSiguiente == NULL )
					goto error;
				segmentoSigueinteReservado = nodoSiguiente->data;/*CORREGIDO*///CORREGIR, NO AVANZA,SIEMPRE ES EL MISMO NODO
				++cantidadDeSegmentosDeDiferencia;
			}
			//me fijo si hubo un siguiente
			if (segmentoSigueinteReservado == NULL )
				goto error;
			//hago memcpy entre el segmento libre y el segmento ocupado
			void* posHacia;
			void* posDesde;
			posHacia = segmento->posMemoria;
			posDesde = segmentoSigueinteReservado->posMemoria;
			memcpy(posHacia, posDesde, segmentoSigueinteReservado->size);
			segmentoSigueinteReservado->pid = -1;
			segmentoSigueinteReservado->segmentoReservado = 0;

			//Re- ajusto el segmento libre y los segmentos libres que pueden llegar a existir en el medio
			if (segmento->size <= segmentoSigueinteReservado->size) {
				//tengo que agrandar el segmento libre
				diferenciaEntreSegmentos = segmentoSigueinteReservado->size
						- segmento->size;

				//agrando el segmento libre
				segmento->size = segmento->size + diferenciaEntreSegmentos;
				//pongo como ocupado al segmento libre
				segmento->segmentoReservado = 1;

				//tomo el siguiente para ver si es el segmentoSiguienteReservado
				nodoSiguiente = nodo->next;/*CORREGIR NODO NEXT ES DEL TIPO TLINKELEMENT */
				segmentoSiguiente = nodoSiguiente->data;
				if (segmentoSiguiente == segmentoSigueinteReservado) {
					//al siguiente lo tengo que achicar y bajar de posinicial, bajar la posmemfisica
					segmento->posInicial =
							segmentoSigueinteReservado->posInicial;
					segmentoSigueinteReservado->posInicial =
							segmentoSigueinteReservado->posInicial
									+ diferenciaEntreSegmentos;
					segmentoSigueinteReservado->posMemoria =
							segmentoSigueinteReservado->posMemoria
									+ diferenciaEntreSegmentos;
					segmentoSigueinteReservado->size =
							segmentoSigueinteReservado->size
									- diferenciaEntreSegmentos;
				} else {//el nodo siguiente al libre es distinto al reservado
						//al siguiente lo tengo que achicar y bajar de posinicial, bajar la posmemfisica

					segmento->posInicial =
							segmentoSigueinteReservado->posInicial;

					//En el caso que el nodo siguiente al libre alcance para guardar el reservado
					if (segmentoSiguiente->size > diferenciaEntreSegmentos) {

						segmentoSiguiente->posInicial =
								segmentoSiguiente->posInicial
										+ diferenciaEntreSegmentos;
						segmentoSiguiente->posMemoria =
								segmentoSiguiente->posMemoria
										+ diferenciaEntreSegmentos;
						segmentoSiguiente->size = segmentoSiguiente->size
								- diferenciaEntreSegmentos;
					}

					if (segmentoSiguiente->size == diferenciaEntreSegmentos) {

						segmentoSiguiente->pid = -1;
						segmentoSiguiente->segmentoReservado = 1;

						bool _condicionParaBorrar(t_segmento* segmento) {
							return segmento->segmentoReservado == 0
									&& segmento->pid == -1
									&& segmento->posInicial
											== segmentoSiguiente->posInicial;
						}

						list_remove_and_destroy_by_condition(UMV,
								(void*) _condicionParaBorrar, free);

					}

					if (segmentoSiguiente->size < diferenciaEntreSegmentos) {
						for (i = 1; i <= cantidadDeSegmentosDeDiferencia; ++i) {

							if (segmentoSiguiente->size
									< segmentoSigueinteReservado->size) {
								diferenciaEntreSegmentos =
										diferenciaEntreSegmentos
												- segmentoSiguiente->size;
								segmentoSiguiente->pid = -1;
								segmentoSiguiente->segmentoReservado = 0;
								bool _condicionParaBorrar(t_segmento* segmento) {
									return segmento->segmentoReservado == 0
											&& segmento->pid == -1
											&& segmento->posInicial
													== segmentoSiguiente->posInicial;
								}

								list_remove_and_destroy_by_condition(UMV,
										(void*) _condicionParaBorrar, free);

								nodoSiguiente = nodo->next;/*CORREGIR NODO NEXT ES DEL TIPO TLINKELEMENT */
								if (nodoSiguiente == NULL )
									goto error;

								segmentoSiguiente = nodoSiguiente->data;
							}
							if (segmentoSiguiente->size
									== diferenciaEntreSegmentos) {
								segmentoSiguiente->pid = -1;
								segmentoSiguiente->segmentoReservado = 1;

								bool _condicionParaBorrar(t_segmento* segmento) {
									return segmento->segmentoReservado == 0
											&& segmento->pid == -1
											&& segmento->posInicial
													== segmentoSiguiente->posInicial;
								}

								list_remove_and_destroy_by_condition(UMV,
										(void*) _condicionParaBorrar, free);
							}

							if (segmentoSiguiente->size
									> diferenciaEntreSegmentos) {
								segmentoSiguiente->posInicial =
										segmentoSiguiente->posInicial
												+ diferenciaEntreSegmentos;
								segmentoSiguiente->posMemoria =
										segmentoSiguiente->posMemoria
												+ diferenciaEntreSegmentos;
								segmentoSiguiente->size =
										segmentoSiguiente->size
												- diferenciaEntreSegmentos;
							}
						}
					}

				}
				//libero el segmento ya que ya hice el memcpy
				segmentoSigueinteReservado->segmentoReservado = 0;
				segmento->segmentoReservado = 1;
				segmento->pid = segmentoSigueinteReservado->pid;
				segmentoSigueinteReservado->pid = -1; //Ya no está más ocupado
				//segmento->posInicial=segmentoSigueinteReservado->posInicial;

			} else {			//como es mas grande, lo fragmento.
				//me fijo la diferencia entre segmentos
				diferenciaEntreSegmentos = segmento->size
						- segmentoSigueinteReservado->size;
				//limito el segmento
				//segmento->size= segmento->size - diferenciaEntreSegmentos;
				fragmentar(diferenciaEntreSegmentos, list_get(UMV, 0),
						segmento);
				/*Debería fragmentar,la funcion que hice no sirve*/

				segmento->segmentoReservado = 1;
				segmento->pid = segmentoSigueinteReservado->pid;
				segmentoSigueinteReservado->segmentoReservado = 0;
				//segmento->posInicial=segmentoSigueinteReservado->posInicial;
			}

			nodo = nodo->next;
			segmento = nodoSiguiente->data;
			++linea;
			//printf("%d\n",linea);
		} else {
			//tomo el siguiente
			nodo = nodo->next;
			if (nodo == NULL )
				goto error;
			segmento = nodo->data;
			//printf("%d\n",linea);
			goto libre;
		}

	}
	error: ;

	void _devolverMemoria(t_segmento* segmento) {
		t_segmento* segmentoCero;
		segmentoCero = list_get(UMV, 0);
		if ((segmento->segmentoReservado == 0) && (segmento->pid == -1)) {
			segmentoCero->size = segmentoCero->size + segmento->size;
		}

	}
	list_iterate(UMV, (void*) _devolverMemoria);

	void _borrarSegmentos(t_segmento* segmento) {

		bool _condicionParaBorrar(t_segmento* segmento) {
			return segmento->segmentoReservado == 0 && segmento->pid == -1;
		}

		list_remove_and_destroy_by_condition(UMV, (void*) _condicionParaBorrar,
				free);

	}
	list_iterate(UMV, (void*) _borrarSegmentos);

	//una vez terminado de compactar, deberia borrar todos los segmentos libres.

//	 list_iterate(UMV, (void*)_comp);

}
