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

extern t_list *UMV;
extern t_log* ptrLog;
//extern int pidActivo;

int compactacion() {

	t_link_element* nodo;
	t_link_element* nodoSiguiente;
	t_link_element* nodoAnterior;
	t_segmento* segmentoLibre;	//segmento libre
	t_segmento* segmentoReservado;	//segmento ocupado
	t_segmento* segmentoSiguiente;
	t_segmento* segmentoCero;
	t_segmento* segmento;
	t_segmento* segmentoSiguienteAlLibre;
	t_segmento* segmentoAnterior;
	pthread_mutex_lock(&escribirListaUMV);

	int diferenciaEntreSegmentos = 0;
	int cantidadDeSegmentosDeDiferencia = 0;
	int i;
	segmentoCero = list_get(UMV, 0);

	if (UMV->elements_count != 1) {
		nodo = UMV->head->next;
	} else
		goto error;
	segmento = nodo->data;

	while (nodo != NULL ) {
		libre: if (segmento->segmentoReservado == 0) {

			//Preparo el ambiente para poder realizar las operaciones

			segmentoLibre = segmento;

			//Busco el primer segmento reservado para subirlo a la posicion del segmento libre

			nodoSiguiente = nodo->next;

			if (nodoSiguiente == NULL )
				goto error;

			segmentoSiguiente = nodoSiguiente->data;
			segmentoSiguienteAlLibre = nodoSiguiente->data;

			//Busco el primer segmento reservado
			while (segmentoSiguiente->segmentoReservado != 1) {
				nodoSiguiente = nodoSiguiente->next;
				if (nodoSiguiente == NULL )
					goto error;
				segmentoSiguiente = nodoSiguiente->data;/*CORREGIDO*///CORREGIR, NO AVANZA,SIEMPRE ES EL MISMO NODO
				++cantidadDeSegmentosDeDiferencia;
			}
			//Encontre el segmento reservado
			segmentoReservado = segmentoSiguiente;

			//Me fijo si el segmento reservado es el siguiente al libre

			if (segmentoReservado == segmentoSiguienteAlLibre) {

				//Si es igual, y el segmento libre es mayor al reservado
				if (segmentoLibre->size > segmentoReservado->size) {
					void* posHacia;
					void* posDesde;
					posHacia = segmentoLibre->posMemoria;
					posDesde = segmentoReservado->posMemoria;
					memcpy(posHacia, posDesde, segmentoReservado->size);
					diferenciaEntreSegmentos = segmentoLibre->size
							- segmentoLibre->size;

					segmentoLibre->size = segmentoLibre->size
							- diferenciaEntreSegmentos;
					segmentoLibre->posInicial = segmentoReservado->posInicial;
					segmentoLibre->segmentoReservado = 1;
					segmentoLibre->pid = segmentoReservado->pid;

					segmentoReservado->posInicial =
							segmentoReservado->posInicial
									- diferenciaEntreSegmentos;
					segmentoReservado->posMemoria =
							segmentoReservado->posMemoria
									- diferenciaEntreSegmentos;

					/*ultima modificacion*/
					segmentoReservado->size = segmentoReservado->size + diferenciaEntreSegmentos;

					segmentoReservado->pid = -1;
					segmentoReservado->segmentoReservado = 0;

				} else {
					//el segmento libre es mas chico que el segmento reservado, debo agrandar el libre y achicar el ocupado

					if (segmentoLibre->size < segmentoReservado->size) {
						void* posHacia;
						void* posDesde;
						posHacia = segmentoLibre->posMemoria;
						posDesde = segmentoReservado->posMemoria;
						memcpy(posHacia, posDesde, segmentoReservado->size);
						diferenciaEntreSegmentos = segmentoReservado->size
								- segmentoLibre->size;

						segmentoLibre->size = segmentoLibre->size
								+ diferenciaEntreSegmentos;
						segmentoLibre->posInicial =
								segmentoReservado->posInicial;
						segmentoLibre->segmentoReservado = 1;
						segmentoLibre->pid = segmentoReservado->pid;

						segmentoReservado->posInicial =
								segmentoReservado->posInicial
										+ diferenciaEntreSegmentos;
						segmentoReservado->posMemoria =
								segmentoReservado->posMemoria
										+ diferenciaEntreSegmentos;
						segmentoReservado->size = segmentoReservado->size
								- diferenciaEntreSegmentos;

						segmentoReservado->pid = -1;
						segmentoReservado->segmentoReservado = 0;

					} else {
						//Los segmentos son iguales
						void* posHacia;
						void* posDesde;
						posHacia = segmentoLibre->posMemoria;
						posDesde = segmentoReservado->posMemoria;
						memcpy(posHacia, posDesde, segmentoReservado->size);

						segmentoLibre->posInicial =
								segmentoReservado->posInicial;
						segmentoLibre->segmentoReservado = 1;
						segmentoLibre->pid = segmentoReservado->pid;

						segmentoReservado->pid = -1;
						segmentoReservado->segmentoReservado = 0;
						segmentoReservado->posInicial =
								segmentoReservado->posInicial + 1;// pos inicial es ilustrativo,
																  //no me dice nada mas que el id del segmento
					}

				}

			} else {
				/*HAY UNO O MAS SEGMENTOS DE DIFERENCIA ENTRE EL LIBRE Y EL RESERVADO
				 * LIBRE LIBRE RESERVADO*/
				//distino al siguiente
				if (segmentoLibre->size > segmentoReservado->size) {
					void* posHacia;
					void* posDesde;
					posHacia = segmentoLibre->posMemoria;
					posDesde = segmentoReservado->posMemoria;
					memcpy(posHacia, posDesde, segmentoReservado->size);
					diferenciaEntreSegmentos = segmentoLibre->size
							- segmentoReservado->size;

					segmentoLibre->size = segmentoLibre->size
							- diferenciaEntreSegmentos;
					segmentoLibre->posInicial = segmentoReservado->posInicial;
					segmentoLibre->segmentoReservado = 1;
					segmentoLibre->pid = segmentoReservado->pid;

					segmentoSiguienteAlLibre->posInicial =  //cambiado
							segmentoSiguienteAlLibre->posInicial
									- diferenciaEntreSegmentos;
					segmentoSiguienteAlLibre->posMemoria =
							segmentoSiguienteAlLibre->posMemoria
									- diferenciaEntreSegmentos;
					segmentoSiguienteAlLibre->size =
							segmentoSiguienteAlLibre->size
									+ diferenciaEntreSegmentos;

					segmentoReservado->pid = -1;
					segmentoReservado->segmentoReservado = 0;

				} else {
					if (segmentoLibre->size == segmentoReservado->size) {
						void* posHacia;
						void* posDesde;
						posHacia = segmentoLibre->posMemoria;
						posDesde = segmentoReservado->posMemoria;
						memcpy(posHacia, posDesde, segmentoReservado->size);

						segmentoLibre->posInicial =
								segmentoReservado->posInicial;
						segmentoLibre->segmentoReservado = 1;
						segmentoLibre->pid = segmentoReservado->pid;

						segmentoReservado->pid = -1;
						segmentoReservado->segmentoReservado = 0;
						segmentoReservado->posInicial =
								segmentoReservado->posInicial + 1;
					} else {
						if (segmentoLibre->size < segmentoReservado->size) {
							//segmentolibre menor al segmento reservado
							//Ya sabemos que debemos ocupar el libre y por lo menos el siguiente al libre
							void* posHacia;
							void* posDesde;
							posHacia = segmentoLibre->posMemoria;
							posDesde = segmentoReservado->posMemoria;
							memcpy(posHacia, posDesde, segmentoReservado->size);
							diferenciaEntreSegmentos = segmentoReservado->size
									- segmentoLibre->size;
							//ocupar el segmento libre
							segmentoLibre->posInicial =
									segmentoReservado->posInicial;
							segmentoLibre->segmentoReservado = 1;
							segmentoLibre->pid = segmentoReservado->pid;
							segmentoLibre->size = segmentoReservado->size;
							//empezar a restar de los segmentos siguientes
							for (i = 0; i <= cantidadDeSegmentosDeDiferencia;//revisar acá
									++i) {
								if (segmentoSiguienteAlLibre->size
										> diferenciaEntreSegmentos/*ultima modificacion*/) {
									//Termino aca y achico
									segmentoSiguienteAlLibre->size =
											segmentoSiguienteAlLibre->size
													- diferenciaEntreSegmentos;
									segmentoSiguienteAlLibre->posInicial =
											segmentoSiguienteAlLibre->posInicial
													+ diferenciaEntreSegmentos;
									segmentoSiguienteAlLibre->posMemoria =
											segmentoSiguienteAlLibre->posMemoria
													+ diferenciaEntreSegmentos;
									segmentoReservado->pid = -1;
									segmentoReservado->segmentoReservado = 0;
									break;

								} else {
									if (segmentoSiguienteAlLibre->size
											< diferenciaEntreSegmentos) {/*ULTIMA MODIFICACION*/
										diferenciaEntreSegmentos =
												diferenciaEntreSegmentos
														- segmentoSiguienteAlLibre->size;

										nodoSiguiente = nodo->next;

										nodoAnterior = nodoSiguiente;
										segmentoAnterior = nodoAnterior->data;

										nodoSiguiente = nodoSiguiente->next;
										segmentoSiguienteAlLibre =
												nodoSiguiente->data;

										bool _destructorSegmentoConsumido(
												t_segmento* segmentoADestruir) {
											return segmentoADestruir->posInicial
													== segmentoAnterior->posInicial
													&& segmentoADestruir->size/*CAMBIE ACA*/
															== segmentoAnterior->size;
										}
										list_remove_and_destroy_by_condition(
												UMV,
												(void*) _destructorSegmentoConsumido,
												free);

										//Lo ocupo y disminuyo la difDeSegmentos,
										//tomo el siguiente para ver si está ocupado , elimino
									} else {					// son iguales
										segmentoReservado->pid = -1;
										segmentoReservado->segmentoReservado =
												0;

										bool _destructorSegmentoConsumido(
												t_segmento* segmentoADestruir) {
											return segmentoADestruir->posInicial
													== segmentoSiguienteAlLibre->posInicial
													&& segmentoADestruir->posInicial
															== segmentoSiguienteAlLibre->size;
										}
										list_remove_and_destroy_by_condition(
												UMV,
												(void*) _destructorSegmentoConsumido,
												free);
										break;
									}

								}

							}

						}
					}
				}

			}
			dump(-1, 0);
			nodo = nodo->next;
			if (nodo == NULL )
				goto error;
			segmento = nodo->data;
		}

		else {
			//tomo el siguiente
			nodo = nodo->next;
			if (nodo == NULL )
				goto error;
			segmento = nodo->data;
			//printf("%d\n",linea);
			goto libre;

		}

	}

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
	pthread_mutex_unlock(&escribirListaUMV);
	return 0;

	error: {

		if (segmento != NULL ) {
			log_debug(ptrLog,
							"Hubo un error en la compactacion, no habia segmentos reservados, pero compacte los libres");
			void _devolverMemoria(t_segmento* segmento) {
				t_segmento* segmentoCero;
				segmentoCero = list_get(UMV, 0);
				if ((segmento->segmentoReservado == 0)
						&& (segmento->pid == -1)) {
					segmentoCero->size = segmentoCero->size + segmento->size;
				}

			}
			list_iterate(UMV, (void*) _devolverMemoria);

			void _borrarSegmentos(t_segmento* segmento) {

				bool _condicionParaBorrar(t_segmento* segmento) {
					return segmento->segmentoReservado == 0
							&& segmento->pid == -1;
				}

				list_remove_and_destroy_by_condition(UMV,
						(void*) _condicionParaBorrar, free);

			}
			list_iterate(UMV, (void*) _borrarSegmentos);
			pthread_mutex_unlock(&escribirListaUMV);
			return 0;
		} else {
			log_debug(ptrLog,
							"Hubo un error en la compactacion, no habia segmentos reservados, no compacte nada");
			pthread_mutex_unlock(&escribirListaUMV);
			return -1;
		}
	}
}
