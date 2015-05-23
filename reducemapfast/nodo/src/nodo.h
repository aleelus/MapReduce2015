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

//Tipos de Emisores de Mensaje
#define ES_JOB 2
#define ES_FS 1
#define ES_NODO 3

//Tipos de Mensajes del Job
#define RECIBIR_ARCHIVO 1
#define NOTIFICACION_NODO 2
#define RECIBIDO_OK 3
//Funciones por Telnet
#define COMANDOBLOQUES 9

//Puerto de escucha del filesystem
int g_Puerto_Fs;

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

//Puerto Escucha del Nodo
int g_Puerto_Nodo;

// - Bandera que controla la ejecución o no del programa. Si está en 0 el programa se cierra.
int g_Ejecutando = 1;

// Retardo (en milisegundos) para contestar una solicitud a un cliente
int g_Retardo = 0;

// Definimos los hilos principales
pthread_t hOrquestadorConexiones, hConsola;

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
