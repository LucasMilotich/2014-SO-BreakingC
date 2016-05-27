/*
 ============================================================================
 Name        : Programa.c
 Author      : Franco Bustos
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>

t_log* CreacionLog();
char* LeerScript(FILE* file);
int EnviarProgramaAlKernel(char *programa);

t_log* Log;
t_config* config;

// Funcion que crea/abre el archivo log del programa
t_log* CreacionLog() {
	char* pathLog = config_get_string_value(config, "LOG");
	t_log* log = log_create(
			pathLog,
			"Programa", 1, LOG_LEVEL_DEBUG);
	return log;
}

//Funcion que lee el contenido del script (.ansisop)
char* LeerScript(FILE* file) {
	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);
	fseek(file, 0, SEEK_SET);
	char *script = malloc(fsize + 1);
	fread(script, fsize, 1, file);
	script[fsize] = '\0';
	log_debug(Log, script);
	return script;
}

int main(int argc, char **argv) {
	//********** Declaracion de variables************
	FILE *file;
	char *script;
	//************************************************

	config = config_create(getenv("ANSISOP_CONFIG"));
	// Se crea el log
	Log = CreacionLog();
	if (Log == NULL)
	{
		log_error(Log, "No se ha podido abrir/crear el archivo Log.txt");
		return EXIT_FAILURE;
	}

	// Se verifica que se reciban todos los parametros
	if (argc < 2)
	{
		log_error(Log, "No han llegado parametros al proceso Programa.");
		return EXIT_FAILURE;
	}

	//********* ARRANCA EL PROGRAMA ***************
	log_info(Log, "Inicio del Programa");
	log_debug(Log, "Abriendo el script..");

	// Se verifica que exista el archivo .ansisop
	if ((file = fopen(argv[1], "r")) == NULL)
	{
		log_error(Log, "No se ha podido abrir el script. Favor, verificar si existe.");
		return EXIT_FAILURE;
	}

	log_debug(Log, "Se abrio el script con exito");

	// Se lee el script (.ansisop)
	log_debug(Log, "Leyendo el script..");
	script = LeerScript(file);
	// Se cierra el archivo .ansisop
	fclose(file);

	// Se envia el contenido del script al Kernel
	log_info(Log, "Enviando programa al kernel");
	log_debug(Log, "Se crea el socket de conexion");
	if (EnviarProgramaAlKernel(script) != 0)
	{
		log_error(Log, "Ocurrio un error durante la conexion con el kernel");
		return EXIT_FAILURE;
	}

	free(script);
	log_destroy(Log);
	config_destroy(config);
	return EXIT_SUCCESS;
}
