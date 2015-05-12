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

// CONSTANTES //
//Ruta del config
#define PATH_CONFIG "config.cfg"
//#define NOMBRE_ARCHIVO_CONSOLA     "Archivo_msp.txt"
#define NOMBRE_ARCHIVO_LOG 		   "nodo.log"

//Tamanio bloques 20mb
#define TAMANIO_BLOQUE 1024*1024*20
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

// Archivo para Espacio de Datos
FILE * archivoEspacioDatos;

//FUNCIONES //
void GrabarBloque();
void CrearArchivoBin();
void mapeo();


// METODOS CONFIGURACION //
void LevantarConfig();

// METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...);

// Logger del commons
t_log* logger;

//Funciones interfaz FileSystem
char* getBloque(int numero);
void setBloque(int numero, char*datos);
char * getFileContent(char* nombre);
