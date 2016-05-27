/*
 * UMV_Main.h
 *
 *  Created on: 05/05/2014
 *      Author: utnso
 */

#ifndef UMV_MAIN_H_
#define UMV_MAIN_H_
#include<pthread.h>


pthread_t UMV_Servidor(void);
pthread_t UMV_Consola(void);

//#define CANT_MEMORIA_TOTAL 1024


//Estructura para el manejo de memoria
typedef struct estructuraDeSegmento{
	//No creamos un campo para segmento, porque usamos el elemcounter de la lista
	int pid;
	int size;
	int posInicial;
	int posMemoria;
	int offset;
	int segmentoReservado; // lo vamos a usar como boolean
					}t_segmento;


#endif /* UMV_MAIN_H_ */
