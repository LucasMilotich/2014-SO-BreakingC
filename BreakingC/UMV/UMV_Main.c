/*
 * UMV_Main.c
 * Programa principal de la UMV
 * Declaración de variables globales y llamado a las funciones principales
 *  Created on: Apr 13, 2014
 *      Author: Breaking C
 */

#include "UMV_Main.h"
#include  <stdio.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdlib.h>
#include <commons/config.h>
#include <signal.h>
#include<pthread.h>
//#include <sockets/Socket.h>


//Defino una lista GLOBAL para administrar la memoria
t_list *UMV;
//Defino el puntero al log global
t_log* ptrLog;
//Defino el proceso activo
int pidActivo;
//Defino el tipo de algoritmo-- Empiezo con FF
int tipoAlgoritmo =1;
//Defino el tiempo de retardo
int tiempoRetardo=0;
//Defino el archivo de configuracion
t_config* config;
//semaforos

pthread_mutex_t escribirListaUMV = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t semaforoTipoProceso= PTHREAD_MUTEX_INITIALIZER;;


int main(void){


//Archivo CFG

char* variableEntorno =getenv( "UMVCFG" );

config = config_create(variableEntorno);
//config = config_create("/home/utnso/tp-2014-1c-breaking-c/BreakingC/UMV/Config.txt");
//config = config_create(variableEntorno);
char* MALLOC= config_get_string_value(config, "MALLOC");
char* LOG= config_get_string_value(config, "LOG");
pthread_t  hiloConsola;
pthread_t  hiloServer;


//FIN Archivo CFG

t_segmento *segmento,*segmentoAux;


ptrLog =log_create(LOG,"UMV",0,0);

log_info(ptrLog,"Se inicializan variables");

//Creo la lista de segmentos
log_info(ptrLog,"El malloc va a ser de: %d",atoi(MALLOC));
UMV=list_create();

//Creo el primer segmento de la memoria, que va a ser donde guardo la memoria total
segmento = malloc(sizeof(t_segmento));
segmento->pid=0;
segmento->posInicial=-1;
segmento->posMemoria=(int)malloc(atoi(MALLOC)); //Creo el "gran malloc" (GM)
segmento->segmentoReservado=1;
segmento->size=atoi(MALLOC);

log_debug(ptrLog,"Creado el bloque de memoria en donde se guarda el GM");
log_debug(ptrLog,"La posicion de memoria inicial del malloc es: %d",segmento->posMemoria);
//Agrego el segmento recien creado a la lista
list_add(UMV,segmento);
log_info(ptrLog,"Se incluyó el primer segmento controlador en la lista de segmentos");
//segmentoAux=UMV->head->data;
segmentoAux=segmento;
/*printf("POSICION DE MEMORIA:%d\n", (segmentoAux->posMemoria));
printf("%d\n", (UMV->elements_count));
printf("%d\n", (UMV->head->data));*/

// Llamo a las funciones para escuchar sockets de entrada y dejar la consola abierta

hiloConsola=UMV_Consola();
log_info(ptrLog,"Se inicializa la consola");
/*hiloServer=*/UMV_Servidor();
log_info(ptrLog,"Se inicializa el servidor");
pthread_join(hiloConsola,NULL);
//pthread_join(hiloServer,NULL);
/*
 *Probando manejo de memoria
p=aux=punteroInicialGM;
printf("La posicion de p:%d\n",p);
*p =2;
printf("Ahora,La posicion de p:%d\n",p);
printf("El valor de p:%d\n",*p);
//p=p+sizeof(int);
p++;
*p=12312;
printf("Ahora,La posicion de p:%d\n",p);
printf("El valor de p:%d\n",*p);

printf("El valor de aux:%d\n",*aux);
printf("La posicion de aux:%d\n",aux);
*/
log_destroy(ptrLog);
config_destroy(config);
free(MALLOC);
free(segmento);
free(config);
free(segmentoAux);
//log_destroy(ptrLog);
return 0;
}


