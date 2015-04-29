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

// CONSTANTES //

#define PATH_CONFIG 			"config.cfg"	//Ruta del config
#define NOMBRE_ARCHIVO_CONSOLA  "Archivo_msp.txt"
#define NOMBRE_ARCHIVO_LOG 		"msp.log"



int g_Puerto_Listen;	//Puerto de escucha
char * g_Lista_Nodos;	//Lista de Nodos necesarios para empezar


// METODOS CONFIGURACION //
void LevantarConfig();

// METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...);

// Logger del commons
//t_log* logger;



