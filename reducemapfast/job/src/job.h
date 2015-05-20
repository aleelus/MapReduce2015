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
#include <netdb.h>

// CONSTANTES //
//Ruta del config
#define PATH_CONFIG "config.cfg"
#define NOMBRE_ARCHIVO_CONSOLA     "Archivo_msp.txt"
#define NOMBRE_ARCHIVO_LOG 		   "msp.log"
#define BUFFERSIZE 10

// METODOS SOCKET //
void conectarMarta();
int EnviarDatos(char*,int);
char* RecibirDatos(char *, int *,int *,int *);

//Ip de Marta
char * g_Ip_Marta;

//Puerto de Marta
char * g_Puerto_Marta;

//Socket Marta
int socket_Marta;

//Programa rutina de mapping
char* g_Mapper;

//Programa rutina de reduce
char* g_Reduce;

//Es combiner
char* g_Combiner;

//Lista de archivos sobre los que se aplica el Job
char* g_Archivos;

//Archivo donde van a estar los resultados
char * g_Resultado;

// METODOS CONFIGURACION //
void LevantarConfig();

// MÉTODOS SOCKETS
void CreoSocket();

// METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...);

// Logger del commons
t_log* logger;
