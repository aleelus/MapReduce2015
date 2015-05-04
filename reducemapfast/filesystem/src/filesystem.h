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

#define PATH_CONFIG 			"config.cfg"	//Ruta del config
#define NOMBRE_ARCHIVO_CONSOLA  "Archivo_msp.txt"
#define NOMBRE_ARCHIVO_LOG 		"msp.log"
#define MAXLINEA				4096			//Maximo de linea de configuracion
#define MAXPUERTO				4				//Maximo puerto

// TIPOS //
struct configuracion {
	int puerto_listen;
	char *lista_nodos;
};


// VARIABLES GLOBALES //
int puerto_listen;								//Puerto de escucha
char lista_nodos;								//Lista de Nodos necesarios para empezar
FILE *config;									//Archivo de configuracion
char linea[MAXLINEA];							//Linea de configuracion
char ch;										//caracter
unsigned int i;									//numero de linea
unsigned int puerto;							//puerto obtenido
struct configuracion configuracion[1] = {{0}};	//Datos de configuracion


// METODOS CONFIGURACION //
//void LevantarConfig();

// METODOS MANEJO DE ERRORES //
//void Error(const char* mensaje, ...);

// Logger del commons
//t_log* logger;



