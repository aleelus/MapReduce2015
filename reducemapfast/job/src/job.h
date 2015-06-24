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


sem_t semaforoLogger;

// CONSTANTES //
//Ruta del config
#define COLOR_VERDE   "\x1b[32m"
#define DEFAULT   "\x1b[0m"
#define PATH_CONFIG "config.cfg"
#define NOMBRE_ARCHIVO_CONSOLA     "Archivo_msp.txt"
#define NOMBRE_ARCHIVO_LOG 		   "msp.log"
#define BUFFERSIZE 50

// METODOS SOCKET //
void conectarMarta();
int EnviarDatos(int,char*,int);
char* RecibirDatos(int ,char *, int *,int *,int *);
void obtenerArrayArchivos(int *);
char * procesarArchivos (char *,int);
char* obtenerRafaga_Uno(char *,char*);
int PosicionDeBufferAInt(char* , int );
int ChartToInt(char );
int ObtenerTamanio (char * ,int, int );
char* abrir_Mapper(char *,char *);
char* obtenerSubBuffer(char *);
char* DigitosNombreArchivo(char *,int *);
int AtiendeCliente(void *);
void CerrarSocket(int);
char* abrir_Reduce(char*,char*);

sem_t semaforo;

typedef struct {
	char *nodo;
	char *ip;
	char *puerto;
	char *bloque;
	char *archResultado;
	char * buffer;
}t_job_a_nodo;

int conectarNodo(t_job_a_nodo , int );
t_job_a_nodo *procesoJob (char *);

char * bufferANodo;
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

//Array de nombres de archivos
char **array_archivos;

//Flag de ejecución
int g_Ejecutando = 1;

// METODOS CONFIGURACION //
void LevantarConfig();

// MÉTODOS SOCKETS
void CreoSocket();

// METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...);

// Logger del commons
t_log* logger;
