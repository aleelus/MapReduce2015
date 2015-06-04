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

#define COLOR_VERDE   "\x1b[32m"
#define DEFAULT   "\x1b[0m"



// CONSTANTES //
//Ruta del config
#define PATH_CONFIG "config.cfg"
//#define NOMBRE_ARCHIVO_CONSOLA     "Archivo_msp.txt"
#define NOMBRE_ARCHIVO_LOG 		   "marta.log"

//Constantes de Manejo de Job
#define RECIBIR_ARCHIVO 1
#define NOTIFICACION_NODO 2
#define RECIBIDO_OK 3

//Tamaño del buffer
#define BUFFERSIZE 10

int socket_fs;

//Puerto de escucha del filesystem
int g_Puerto;

//Ip del filesystem
char * g_Ip_Fs;

//Puerto Fs
char * g_Puerto_Fs;

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

//Lista de Archivos
t_list *lista_archivos;

//Array de Listas para funcion magica
t_list **array_listas;

//Lista de Nodos
t_list *lista_nodos;

// METODOS CONFIGURACION //
void LevantarConfig();

// METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...);

// Logger del commons
t_log* logger;

//Contador de Hilos
int cantHilos=0;

// Definimos los hilos principales
pthread_t hOrquestadorConexiones, hConsola;

//METODOS MANEJO SOCKETS
void HiloOrquestadorDeConexiones();

char* obtenerSubBuffer(char *);
void conectarAFileSystem();
void FuncionMagica(t_list* );

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
#define COMANDO 9
#define COMANDOBLOQUES 8
#define COMANDONODOS 7

//ESTRUCTURAS//
typedef struct{
	char*nodo;
	char*bloque;
	char *archivo;
	int estado;
}t_job_enviado;

t_list *lista_job_enviado;


static t_job_enviado *job_enviado_create(char *nodo, char *bloque,char* archivo) {
	t_job_enviado *new = malloc(sizeof(t_job_enviado));
	new->nodo=strdup(nodo);
	new->bloque=strdup(bloque);
	new->archivo=strdup(archivo);
	new->estado=0;
	return new;
}

//Estructura de array para funcion magica
typedef struct{
	char*dato;
	char*bloqueDelNodo;
	int peso;
}t_dato;

static t_dato *dato_create(char *dato, char*bloqueDelNodo) {
	t_dato *new = malloc(sizeof(t_dato));
	new->dato = strdup(dato);
	new->bloqueDelNodo= strdup(bloqueDelNodo);
	new->peso=0;
	return new;
}

//Estructura de Array Copias
typedef struct{
    char *nodo;
    char *bloque;
    int estado;
}t_array_copias;

//Estructura Lista de Bloques
typedef struct{
    char *bloque;
    t_array_copias array[3];

}t_bloque;



static t_bloque *bloque_create(char *bloque, t_array_copias *array) {
	t_bloque *new = malloc(sizeof(t_bloque));
	new->bloque = strdup(bloque);
	new->array[0] = array[0];
	new->array[1] = array[1];
	new->array[2] = array[2];
	return new;
}

//Estructura Lista de Archivos
typedef struct{
    char *nombreArchivo;
    char *nombreArchivoResultado;
    int tieneCombiner;
    int idJob;
    t_list *listaBloques;
    t_list **array_de_listas;
}t_archivo;


t_bloque* buscarNodoYBloque (char * , char *,int*,t_archivo **);

static t_archivo *archivo_create(char *nombreArchivo, int id) {
	t_archivo *new = malloc(sizeof(t_archivo));
	new->idJob = id;
	new->nombreArchivo = strdup(nombreArchivo);
	new->nombreArchivoResultado="";
	new->tieneCombiner=0;
	new->listaBloques= list_create();
	return new;
}

typedef struct{
    char *bloque;
    char *archivo;
    int procesando;
}t_bloqueArchivo;

static t_bloqueArchivo *bloqueArchivo_create(char *bloque, char *archivo, int procesando) {
	t_bloqueArchivo *new = malloc(sizeof(t_bloqueArchivo));
	new->bloque = strdup(bloque);
	new->archivo = strdup(archivo);
	new->procesando= procesando;
	return new;
}


typedef struct {
	//Estructura Lista de Nodos
	    char *nombreNodo;
	    char *ipNodo;
	    char *puertoNodo;
	    int estado;
	    t_list *listaBloqueArchivo;
	    int cantTareasPendientes;
	    char *tarea;

} t_nodo;

static t_nodo *nodo_create(char *nombreNodo, char *ipNodo, char* puertoNodo) {
	t_nodo *new = malloc(sizeof(t_nodo));
	new->nombreNodo = strdup(nombreNodo);
	new->ipNodo = strdup(ipNodo);
	new->puertoNodo = strdup(puertoNodo);
	new->estado = 1;
	new->listaBloqueArchivo= list_create();
	new->cantTareasPendientes=0;
	new->tarea = "";

	return new;
}

// El tipo de Mensaje para el Job
#define ES_UN_SOLO_ARCHIVO 1
#define ES_MUCHOS_ARCHIVOS 2
