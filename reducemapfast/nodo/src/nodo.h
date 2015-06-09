// Bibliotecas //
#include <sys/mman.h>
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
#include <limits.h>	/* for PATH_MAX */
#include <netdb.h>
#include <signal.h>

// CONSTANTES //
//Ruta del config
#define PATH_CONFIG "config.cfg"

//#define NOMBRE_ARCHIVO_CONSOLA     "Archivo_msp.txt"
#define NOMBRE_ARCHIVO_LOG 		   "nodo.log"

//Tamanio bloques 20mb
#define TAMANIO_BLOQUE 1024*1024*20

//Tipos de Mensaje segun Interface
#define GET_BLOQUE 2
#define SET_BLOQUE 3
#define GET_FILE_CONTENT 4

//Tipos de Emisores de Mensaje
#define ES_JOB 2
#define ES_FS 1
#define ES_NODO 3

//Tipos de Mensajes del Job
#define RECIBIR_TRABAJO 1
#define NOTIFICACION_NODO 2
#define RECIBIDO_OK 3

//Tipos de Mensaje de FS
#define SOLICITUD_DE_CONEXION 1

//Funciones por Telnet
#define COMANDOBLOQUES 9

//Puerto de escucha del filesystem
char * g_Puerto_Fs;

//Ip del filesystem
char * g_Ip_Fs;

//Archivo con los bloques
char* g_Archivo_Bin;

//Tamanio archivo de bloques
int g_Tamanio_Bin;

//Directorio Temporal
char* g_Dir_Temp;

//Nodo Nuevo
char* g_Nodo_Nuevo;

//Ip del Nodo
char* g_Ip_Nodo;

//Socket de FS
int socket_Fs;

//Puerto Escucha del Nodo
int g_Puerto_Nodo;

// - Bandera que controla la ejecución o no del programa. Si está en 0 el programa se cierra.
int g_Ejecutando = 1;

// Retardo (en milisegundos) para contestar una solicitud a un cliente
int g_Retardo = 0;

//Definimos los hilos principales
pthread_t hOrquestadorConexiones, hConsola;

//Variable Global del Estado de Nodo 1=Ocupado 0=Desocupado
int estado=0;

//Variable Global que indica si ya estuvo conectado al FS
int conectado=0;

//METODOS MANEJO SOCKETS
void HiloOrquestadorDeConexiones();

// Tamanio del buffer
#define BUFFERSIZE 1024*4 //Definir tamanio buffer

//Mensajes aceptados
#define MSJ_LEER_BLOQUE          1
#define MSJ_ESCRIBIR_BLOQUE      2
#define MSJ_GET_TEMP             3

// Archivo para Espacio de Datos
FILE * archivoEspacioDatos;

//FUNCIONES //
void GrabarBloque();
void mapeo();
int conectarFS(int*_Fs,char*,char*);
void conexionAFs();
char* RecibirDatos(int, char *, int *,int *,int *);
int EnviarDatos(int , char *, int);
int runScriptFile(char* script,char* archNom, char* input);
void grabarScript(char* nombreScript, char* codigoScript);

// METODOS CONFIGURACION //
void LevantarConfig();

// METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...);

//Mensaje de error global.
char* g_MensajeError;

// Logger del commons
t_log* logger;

int pagina;  //Tamanio paginas Mapeo

//Funciones interfaz FileSystem
char* getBloque(int numero);
void setBloque(int numero, char*datos);
char * getFileContent(char* nombre);

//Estructura Datos del Job
typedef struct{
    char *nombreSH;
    char *contenidoSH;
    char *bloque;
    char *nombreResultado;
}t_job;

static t_job *job_create(char *nSH, char* contenidoSH, char* el_Bloque, char * nResultado) {
	t_job *new = malloc(sizeof(t_job));
	new->nombreSH = strdup(nSH);
	new->contenidoSH = strdup(contenidoSH);
	new->bloque = strdup(el_Bloque);
	new->nombreResultado = strdup(nResultado);
	return new;
}

//Defines de los pipes
/* since pipes are unidirectional, we need two pipes.
   one for data to flow from parent's stdout to child's
   stdin and the other for child's stdout to flow to
   parent's stdin */

#define NUM_PIPES          2
#define PARENT_WRITE_PIPE  0
#define PARENT_READ_PIPE   1
int pipes[NUM_PIPES][2];

/* always in a pipe[], pipe[0] is for read and
   pipe[1] is for write */
#define READ_FD  0
#define WRITE_FD 1
#define PARENT_READ_FD  ( pipes[PARENT_READ_PIPE][READ_FD]   )
#define PARENT_WRITE_FD ( pipes[PARENT_WRITE_PIPE][WRITE_FD] )
#define CHILD_READ_FD   ( pipes[PARENT_WRITE_PIPE][READ_FD]  )
#define CHILD_WRITE_FD  ( pipes[PARENT_READ_PIPE][WRITE_FD]  )
