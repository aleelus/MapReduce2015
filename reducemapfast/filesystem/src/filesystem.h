// Bibliotecas //
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <commons/config.h>
#include <string.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <semaphore.h>
#include <string.h>
#include <netdb.h>

// CONSTANTES //

#define PATH_CONFIG 			"/home/gabriel/workspace/sisop/tp-2015-1c-los-barderos/reducemapfast/filesystem/src/config.cfg"	//Ruta del config
#define NOMBRE_ARCHIVO_CONSOLA  "Archivo_msp.txt"
#define NOMBRE_ARCHIVO_LOG 		"msp.log"
#define MAXLINEA				4096			//Maximo de linea de configuracion
#define MAXPUERTO				4				//Maximo puerto

// TIPOS //
struct configuracion {
	unsigned puerto_listen;
	char *lista_nodos;
};


// VARIABLES GLOBALES //



// METODOS CONFIGURACION //
void LevantarConfig();

// METODOS MANEJO DE ERRORES //
//void Error(const char* mensaje, ...);

// Logger del commons
//t_log* logger;



