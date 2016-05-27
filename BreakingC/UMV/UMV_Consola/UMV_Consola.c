#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<commons/collections/list.h>
#include "../estructuras.h"
#include "../UMV_Main.h"
#include <commons/log.h>
#include "../funcionesDeUMV.h"
#include <commons/config.h>
#include <commons/txt.h>
extern t_log* ptrLog;
extern t_list* UMV;

int contadorSegmento = 0;
extern int tipoAlgoritmo;

void *consola();
void dump(int, int);
void retardo(int);
void cambiarAlgoritmo(int);

pthread_t UMV_Consola(void) {
	pthread_t HILOconsola;

	if ((pthread_create(&HILOconsola, NULL, consola, NULL )) < 0) {
		perror("could not create thread");
		return 1;
	}

	return HILOconsola;

}

void *consola() {
	int c, i;
	char* tipo_de_operacion;
	int posArgumento;
	int cantidadDePalabras = 0;
	int j, flag;
	int base = 0;
	int pidOOffset;
	int tamanio;
	flag = 1;
	log_info(ptrLog, "Se inicializan variables de la consola");
	while (flag != 0) {
		char* comandos = calloc(sizeof(char) * 1000, sizeof(char));

		int* argumentos = calloc(sizeof(int) * 6, sizeof(char)); //0proceso,1base,2offset,3tamanio,4tiempo_retardo,5tipoalgoritmo
		log_info(ptrLog, "Dejo la consola a la espera de escritura");
		//Adquiero el comando con parametros
		for (i = 0; (c = getchar()) != '\n'; ++i) {
			comandos[i] = c;
			if (c == ' ')
				++cantidadDePalabras;
		}
		comandos[i] = '\0';

		printf("Lo que ingresaste es : %s\n", comandos);

		//Lo analizo - PASAR A BIBLIOTECA Z
		i = 0;
		while (comandos[i] != '\0' && comandos[i] != 32) {
			++i;
		}
		//Le asigno mem al vec que va a tener el comando a ejecutar
		tipo_de_operacion = (char*) calloc(i + 1, sizeof(char));
		tipo_de_operacion[i] = '\0';
		for (i = i - 1; i > -1; --i) {
			tipo_de_operacion[i] = comandos[i];
		}

		//OPERACION
		if (strcmp(tipo_de_operacion, "operacion") == 0
				|| strcmp(tipo_de_operacion, "Operacion") == 0) {
			log_info(ptrLog, "Estoy dentro de operacion ");
			posArgumento = 0;
			i = i + 1; //Antes i apuntaba al ' ', ahora lo hago apuntar al proceso
			while (tipo_de_operacion[i] != '\0')
				// posicion el i en la proxima palabra (1er argumento), queda en ' '
				++i;
			// Lo dejo en el primer argumento
			++i;

			/*	Cuento la cantidad de palabras
			 * 	cree esto cantidadDePalabras, que cuenta cada ' '
			 * 	luego dependiendo de ese valor voy a elegir la operacion
			 * 	luego voy a recorrer cada comando
			 */

			while (comandos[i] != '\0' && comandos[i] != '\n'
					&& posArgumento < 4) {

				//Proceso en este orden  proceso,base,offset,tamanio ;;
				//el buffer almacena 10 valores
				/*	for(;comandos[i]!=32;++i);
				 ++i;*/
				char* BufferParaConvertirAInt = calloc(sizeof(char) * 11,
						sizeof(char));
				for (j = 0; comandos[i] != 32 && comandos[i] != '\0'; ++i) {
					BufferParaConvertirAInt[j] = comandos[i];
					++j;
				}

				++i;
				argumentos[posArgumento] = atoi(BufferParaConvertirAInt);
				free(BufferParaConvertirAInt);
				++posArgumento;

				//llamar a funciones para realizar x cosa, con sus argumentos dentro del vector argumentos

			}

			t_solicitarBytes solicitarByte;
			t_crear_segmento crearSegment;
			t_destruir_segmento destruirSegment;
			t_enviarBytes enviarByte;
			switch (cantidadDePalabras) {
			case 1:/*Destruir segmento*/
				log_info(ptrLog, "Entrando a destruir segmento consola");
				destruirSegment.programID = argumentos[0];
				if (destruirSegmento(&destruirSegment) == 0) {
					log_info(ptrLog, "Segmento %d borrado",
							destruirSegment.programID);
					printf("Segmento %d borrado\n", destruirSegment.programID);
				} else {
					log_debug(ptrLog, "Segmento %d NO borrado",
							destruirSegment.programID);
					printf("Segmento %d NO borrado\n",
							destruirSegment.programID);
				}

				break;
			case 2:/*Crear segmento*/
				log_info(ptrLog, "Entrando a crear segmento consola");
				crearSegment.programID = argumentos[0];
				crearSegment.tamanio = argumentos[1];
				base = crearSegmento(&crearSegment);
				printf(
						"Se creo el siguiente segmento %d para el programa %d con tamanio %d\n",
						base, crearSegment.programID, crearSegment.tamanio);
				log_debug(ptrLog,
						"Se creo el siguiente segmento %d para el programa %d con tamanio %d\n",
						base, crearSegment.programID, crearSegment.tamanio);

				break;
			case 4:/*SOLICITAR BYTES*/
				log_info(ptrLog, "En solicitarBytes en consola");
				solicitarByte.base = argumentos[1];
				solicitarByte.offset = argumentos[2];
				solicitarByte.tamanio = argumentos[3];
				char* bufferRespuesta /*= calloc(solicitarByte.tamanio,
				 sizeof(char))*/;
				bufferRespuesta = solicitarBytes(&solicitarByte,
						&(argumentos[0]));
				if (bufferRespuesta[0] == -1) {
					printf("Hubo un error\n");
					log_error(ptrLog, "Hubo un error en solicitarBytes");
				} else {
					extern t_config* config;
					FILE* dump;
					char* DUMP = config_get_string_value(config, "DUMP");
					dump = txt_open_for_append(DUMP);
					printf("Lo solicitado es: ");
					fprintf(dump, "\n Lo solicitado de enviar bytes es: \n");
					for (i = 0; i < solicitarByte.tamanio; ++i) {
						printf("%x ", bufferRespuesta[i]);
						fprintf(dump, "%x ", bufferRespuesta[i]);
						fflush(dump);
					}
					//free(DUMP);
					free(bufferRespuesta);
					fclose(dump);
					printf(".\n");
					log_info(ptrLog,
							"Se completo correctamente la operacion solicitarBytes de consola");
				}

				printf(
						"Operacion con parametros PID: %d BASE: %d OFFSET: %d SIZE: %d\n",
						argumentos[0], argumentos[1], argumentos[2],
						argumentos[3]);
				log_info(ptrLog, "Fin de solicitarByte en consola");

				break;

			case 5:/*Enviar Bytes*/
				log_info(ptrLog, "En enviarBytes en consola");
				enviarByte.base = argumentos[1];
				enviarByte.offset = argumentos[2];
				enviarByte.tamanio = argumentos[3];
				enviarByte.buffer = calloc(enviarByte.tamanio, sizeof(char));
				/*Verifico si es stream*/
				if (comandos[i] == 39 /*apostrofe, indica caracter*/) {
					//Le sumo uno para que el buffer, que sirve de argumento, tenga a partir del primer caracter
					++i;
					for (j = 0; j < enviarByte.tamanio && comandos[i] != '\0';
							++i) {

						enviarByte.buffer[j] = comandos[i];
						++j;
					}
					//++i;
					if (comandos[i] == 39 /*apostrofe, indica fin caracter*/)
						;
					else {
						printf("Falto cerrar stream con el caracter '(39)");
						log_error(ptrLog,
								"Hubo un error en enviarByte de consola, no se cerró el stream con el caracter ' (39)");
						break;
					}
				}

				/*Verifico si es string*/
				if (comandos[i] == 34 /*comilla, indica comienzo string*/) {
					//Le sumo uno para que el buffer, que sirve de argumento, tenga a partir del primer caracter
					++i;
					for (j = 0;
							j < enviarByte.tamanio - 1 && comandos[i] != '\0';
							++i) {

						enviarByte.buffer[j] = comandos[i];
						++j;
					}
					//	++j;
					enviarByte.buffer[j] = '\0'; //Cierro string
					//	++i;
					if (comandos[i] == 34 /*comilla, indica fin caracter*/)
						;
					else {
						printf(
								"Falto cerrar string  con el caracter comilla(34)");
						log_error(ptrLog,
								"Hubo un error en enviarByte de consola, no se cerró el string con el caracter '' (34)");
						break;
					}

				}

				if (comandos[i] != 34 && comandos[i] != 39) {/*lo tomo como numero*/
					memcpy(enviarByte.buffer, comandos + i, enviarByte.tamanio);
				}

				char* c = malloc(1);
				c = enviarBytes(&enviarByte, (argumentos[0]));
				if (c[0] == -1) {
					printf("Hubo un error\n");
					log_error(ptrLog, "Hubo un error en enviarByte de consola");
				} else
					printf("Operacion satisfactoria\n");

				log_info(ptrLog, "Fin de enviar Bytes");

				break;

			}

		}

		//RETARDO
		if (strcmp(tipo_de_operacion, "retardo") == 0
				|| strcmp(tipo_de_operacion, "Retardo") == 0) {
			log_info(ptrLog, "Estoy dentro de retardo ");
			posArgumento = 4;
			i = i + 1; //Antes i apuntaba al ' ', ahora lo hago apuntar al tiempo de retardo
			while (tipo_de_operacion[i] != '\0')
				// posicion el i en la proxima palabra (1er argumento)
				++i;
			//voy al primer argumento
			++i;
			while (comandos[i] != '\0' && comandos[i] != '\n') {
				char* BufferParaConvertirAInt = calloc(sizeof(char) * 11,
						sizeof(char));
				for (j = 0; comandos[i] != 32 && comandos[i] != '\0'; ++i) {
					BufferParaConvertirAInt[j] = comandos[i];
					++j;
				}

				argumentos[posArgumento] = atoi(BufferParaConvertirAInt);
				free(BufferParaConvertirAInt);
				retardo(argumentos[posArgumento]);

			}
			printf("Retardo de %d aplicado.\n", argumentos[posArgumento]);
			log_info(ptrLog, "Se aplico un retardo de %d ",argumentos[posArgumento] );
		}

		//ALGORITMO
		if (strcmp(tipo_de_operacion, "algoritmo") == 0
				|| strcmp(tipo_de_operacion, "Algoritmo") == 0) {
			log_info(ptrLog,"Estoy dentro de comando algoritmo en consola");
			posArgumento = 5;

			i = i + 1; //Antes i apuntaba al ' ', ahora lo hago apuntar al tiempo de retardo
			while (tipo_de_operacion[i] != '\0')
				// posicion el i en la proxima palabra (1er argumento), ' '
				++i;
			//voy al primer argumento
			++i;
			j = 0;
			char* BufferParaConvertirAInt = calloc(sizeof(char) * 11,
					sizeof(char));
			while (comandos[i] != '\0') {
				BufferParaConvertirAInt[j] = comandos[i];
				++j;
				++i;
			}
			BufferParaConvertirAInt[j] = '\0';
			++i;

			if (strcmp(BufferParaConvertirAInt, "First Fit") == 0
					|| strcmp(BufferParaConvertirAInt, "first fit") == 0
					|| strcmp(BufferParaConvertirAInt, "FirstFit") == 0
					|| strcmp(BufferParaConvertirAInt, "firstfit") == 0) {
				argumentos[posArgumento] = 1;
				cambiarAlgoritmo(argumentos[posArgumento]);
				log_info(ptrLog,
						"Se corrio exitosamente el cambio de algoritmo a first fit en consola");
				printf("Se aplico correctamente el algoritmo first fit\n");
			}

			else
				//printf("No es FF\n")
				;
			if (strcmp(BufferParaConvertirAInt, "Worst Fit") == 0
					|| strcmp(BufferParaConvertirAInt, "worst fit") == 0
					|| strcmp(BufferParaConvertirAInt, "WorstFit") == 0
					|| strcmp(BufferParaConvertirAInt, "worstfit") == 0) {
				argumentos[posArgumento] = 0;
				cambiarAlgoritmo(argumentos[posArgumento]);
				log_info(ptrLog,
						"Se corrio exitosamente el cambio de algoritmo a worst fit en consola");
				printf("Se aplico correctamente el algoritmo worst fit\n");
			} else
				//printf("No es WF\n")
				;
			free(BufferParaConvertirAInt);

		}
		//COMPACTACION
		if (strcmp(tipo_de_operacion, "compactacion") == 0
				|| strcmp(tipo_de_operacion, "Compactacion") == 0) {
			//compactar();
			if (compactacion() == 0) {
				log_info(ptrLog, "Se ejecutó correctamente la compactacion");
				printf("Se generó la compactación correctamente.\n");
			} else {
				log_error(ptrLog, "Se ejecutó con error la compactacion");
				printf("Se generó la compactación con error.\n");
			}
		}
		//DUMP
		if (strcmp(tipo_de_operacion, "dump") == 0
				|| strcmp(tipo_de_operacion, "Dump") == 0) {
			log_info(ptrLog,"Se va a realizar el dump");
			i = 0;
			while (tipo_de_operacion[i] != '\0')
				// posicion el i en la proxima palabra (1er argumento), ' '
				++i;
			//voy al primer argumento
			++i;
			posArgumento = 0;
			while (comandos[i] != '\0' && comandos[i] != '\n'
					&& posArgumento < cantidadDePalabras) {

				char* BufferParaConvertirAInt = calloc(sizeof(char) * 11,
						sizeof(char));
				for (j = 0; comandos[i] != 32 && comandos[i] != '\0'; ++i) {
					BufferParaConvertirAInt[j] = comandos[i];
					++j;
				}

				++i;
				argumentos[posArgumento] = atoi(BufferParaConvertirAInt);
				free(BufferParaConvertirAInt);
				++posArgumento;

				//llamar a funciones para realizar x cosa, con sus argumentos dentro del vector argumentos

			}
			if (cantidadDePalabras == 0) {
				pidOOffset = -1;
				tamanio = 0;
			}

			if (cantidadDePalabras == 1) {
				pidOOffset = argumentos[0];
				tamanio = 0;
			}

			if (cantidadDePalabras == 2) {
				pidOOffset = argumentos[0];
				tamanio = argumentos[1];
			}

			dump(pidOOffset, tamanio);
			log_info(ptrLog, "Se ejecuto el DUMP");
			printf("Se imprimio el archivo.\n");
		}
		//SALIR
		if (strcmp(tipo_de_operacion, "salir") == 0
				|| strcmp(tipo_de_operacion, "Salir") == 0) {
			//llamar Fx
			printf("Saliste de la consola!\n");
			flag = 0;
		}
		cantidadDePalabras = 0;
		free(comandos);
		//free(BufferParaConvertirAInt);
		free(argumentos);
		free(tipo_de_operacion);
	}

	return 0;
}

void dump(int pidOOffset, int tamanio) {
	int pidactivo;
	/*IMPRESION POR PANTALLA A COMENTAR
	void _imprimirSegmento1(t_segmento* segmento) {
		extern int contadorSegmento;
		printf("%d ", contadorSegmento);
		printf(" PID: %d", segmento->pid);
		printf(" BASE: %d", segmento->posInicial);
		printf(" SIZE: %d", segmento->size);
		printf(" PosMemFi: %d\n", segmento->posMemoria);
		++contadorSegmento;
	}
	bool _buscaPID1(t_segmento* segmento) {
		return segmento->pid != -100;
	}

	t_list* listaDeSegmentos1 = list_filter(UMV, (void*) _buscaPID1);

	list_iterate(listaDeSegmentos1, (void*) _imprimirSegmento1);

	/*###########################################################################
	contadorSegmento = 0;

	########### IMPRESION ARCHIVO DUMP ################
	list_destroy(listaDeSegmentos1);*/
	extern t_config* config;
	//char* DUMP = config_get_string_value(config, "DUMP");
	FILE* dump;
	t_segmento* segmentoCero = list_get(UMV, 0);
	char* DUMP = config_get_string_value(config, "DUMP");
	dump = txt_open_for_append(DUMP);
	//dump = txt_open_for_append(
	//	"/home/utnso/tp-2014-1c-breaking-c/BreakingC/UMV/Dump.txt");
	// txt_write_in_file(dump, "prueba");

	if (pidOOffset == -1 && tamanio == 0) {
		fprintf(dump,
				"\n\n################ Memoria Libre = %d ################",
				segmentoCero->size);
		fflush(dump);

		void _imprimirSegmento(t_segmento* segmento) {
			extern int contadorSegmento;
			fprintf(dump, "\n%d", contadorSegmento);
			fflush(dump);
			if (segmento->pid == -1) {
				fprintf(dump, "       PID:         Libre");
				fflush(dump);
			} else {
				fprintf(dump, "       PID:         %d", segmento->pid);
				fflush(dump);
			}
			fprintf(dump, "       BASE:        %d", segmento->posInicial);
			fflush(dump);
			fprintf(dump, "       SIZE:        %d", segmento->size);
			fflush(dump);
			fprintf(dump, "       PosMemFi:    %d", segmento->posMemoria);
			fflush(dump);
			++contadorSegmento;
		}
		bool _buscaPID(t_segmento* segmento) {
			if (pidOOffset == -1 && tamanio == 0 && segmento->posInicial != -1)
				return segmento->pid != -100 && segmento->posInicial != -1;
			if (pidOOffset != -1 && tamanio == 0)
				return segmento->pid == pidOOffset;
			else
				return false;
		}

		t_list* listaDeSegmentos = list_filter(UMV, (void*) _buscaPID);
		fprintf(dump,
				"\n########################## LISTA DE SEGMENTOS  ###############################\n");
		list_iterate(listaDeSegmentos, (void*) _imprimirSegmento);
		list_destroy(listaDeSegmentos);
	}

	if (pidOOffset >= 0 && tamanio > 0) {
		//imprio el solicitarBytes
		int i;
		t_solicitarBytes enviarbyte;
		enviarbyte.base = 0;
		enviarbyte.offset = pidOOffset;
		enviarbyte.tamanio = tamanio;
		pidactivo = 0;
		char* buffer = calloc(tamanio, sizeof(char));
		buffer = solicitarBytes(&enviarbyte, &pidactivo);
		fprintf(dump, "\n El dato que se pidio es: ");
		fflush(dump);
		for (i = 0; i <= tamanio; ++i) {
			fprintf(dump, "%x", buffer[i]);
			fflush(dump);

		}

		free(buffer);

	}

	if (pidOOffset >= 1 && tamanio == 0) {
		fprintf(dump, "\n Lista de segmentos para el programa: %d", pidOOffset);
		void _imprimirSegmento(t_segmento* segmento) {
			extern int contadorSegmento;
			fprintf(dump, "\n%d", contadorSegmento);
			fflush(dump);
			fprintf(dump, "       BASE:        %d", segmento->posInicial);
			fflush(dump);
			fprintf(dump, "       SIZE:        %d", segmento->size);
			fflush(dump);
			fprintf(dump, "       PosMemFi:    %d", segmento->posMemoria);
			fflush(dump);
			++contadorSegmento;
		}
		bool _buscaPID(t_segmento* segmento) {
			if (pidOOffset == -1 && tamanio == 0 && segmento->posInicial != -1)
				return segmento->pid != -100 && segmento->posInicial != -1;
			if (pidOOffset != -1 && tamanio == 0)
				return segmento->pid == pidOOffset;
			else
				return false;
		}

		t_list* listaDeSegmentos = list_filter(UMV, (void*) _buscaPID);

		list_iterate(listaDeSegmentos, (void*) _imprimirSegmento);
		list_destroy(listaDeSegmentos);
	}
	fclose(dump);
	//free(DUMP);
	/*###########################################################################*/

}

void retardo(int tiempo) {

	extern int tiempoRetardo;
	tiempoRetardo = tiempo / 1000;

}

void cambiarAlgoritmo(int Algoritmo) {
	tipoAlgoritmo = Algoritmo;

}
