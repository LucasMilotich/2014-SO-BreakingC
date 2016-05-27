#include "kernel.h"

t_log* creacionLog(char * ruta, char* nombrePrograma) {
	t_log* log = log_create(ruta,nombrePrograma, 1, 1);
	return log;
}
