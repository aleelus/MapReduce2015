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
//Ruta del config
#define PATH_CONFIG "config.cfg"
//#define NOMBRE_ARCHIVO_CONSOLA     "Archivo_msp.txt"
#define NOMBRE_ARCHIVO_LOG 		   "marta.log"

//Tamaño del buffer
#define BUFFERSIZE 10

//Puerto de escucha del filesystem
int g_Puerto;

//Ip del filesystem
char * g_Ip_Fs;

//Archivo con los bloques
char* g_Archivo_Bin;

//Directorio Temporal
char* g_Dir_Temp;

//Nodo Nuevo
char* g_Nodo_Nuevo;

//Ip del Nodo
char* g_Ip_Nodo;

//Puerto Escucha del Nodo
int g_Puerto_Nodo;

// METODOS CONFIGURACION //
void LevantarConfig();

// METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...);

// Logger del commons
t_log* logger;

// Definimos los hilos principales
pthread_t hOrquestadorConexiones, hConsola;

//METODOS MANEJO SOCKETS
void HiloOrquestadorDeConexiones();

// - Bandera que controla la ejecución o no del programa. Si está en 0 el programa se cierra.
int g_Ejecutando = 1;

//Longitud del Buffer
int longitudBuffer;

//contador de ID JOB (semaforo)
int id_job=0;

//Mensajes aceptados
//#define MSJ_SALUDO          1
#define ES_JOB	2
#define ES_FS	1

//ESTRUCTURAS//
//Estructura de Array Copias
typedef struct{
    char *nodo;
    char *bloque;
}t_array_copias;

//Estructura Lista de Bloques
typedef struct{
    char *bloque;
    t_array_copias array[3];
    struct t_bloque *next;
}t_bloque;

static t_bloque *bloque_create(char *bloque, t_array_copias array) {
	t_bloque *new = malloc(sizeof(bloque));
	new->bloque = strdup(bloque);
	new->array[0].bloque="";
	new->array[0].nodo="";
	new->array[1].bloque="";
	new->array[1].nodo="";
	new->array[2].bloque="";
	new->array[2].nodo="";
	new->next=NULL;
	return new;
}

//Estructura Lista de Archivos
typedef struct{
    char *nombreArchivo;
    char *nombreArchivoResultado;
    int tieneCombiner;
    int idJob;
    t_bloque *listaBloques;
    struct t_archivo *next;
}t_archivo;

static t_archivo *archivo_create(char *nombreArchivo, t_bloque *listaBloques) {
	t_archivo *new = malloc(sizeof(t_archivo));
	new->nombreArchivo = strdup(nombreArchivo);
	new->nombreArchivoResultado="";
	new->tieneCombiner=0;
	new->listaBloques= NULL;
	new->next=NULL;
	return new;
}


typedef struct {
	//Estructura Lista de Nodos
	    char *nombreNodo;
	    char *ipNodo;
	    int puertoNodo;
	    int estado;
	    int procesando;
	    char *nombreArchivo;
	    char *bloqueArchivo;
	    char *tarea;
} t_nodo;

static t_nodo *nodo_create(char *nombreNodo, char *ipNodo, int puertoNodo) {
	t_nodo *new = malloc(sizeof(t_nodo));
	new->nombreNodo = strdup(nombreNodo);
	new->ipNodo = strdup(ipNodo);
	new->puertoNodo = puertoNodo;
	new->estado = 1;
	new->procesando = 0;
	new->nombreArchivo = "";
	new->bloqueArchivo = "";
	new->tarea = "";
	return new;
}

// El tipo de Mensaje para el Job
#define ES_UN_SOLO_ARCHIVO 1
#define ES_MUCHOS_ARCHIVOS 2
