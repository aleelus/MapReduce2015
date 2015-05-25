// Bibliotecas //
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <semaphore.h>
//#include <bson.h>
//#include <mongoc.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>



//#define IP 						"127.0.0.1"
//#define PUERTO					"6667"
//#define PACKAGESIZE				1024
//#define CANTMAX					1			//Cantidad maxima para seleccionar un comando
#define PATH_CONFIG 			"config.cfg"	//Ruta del config
#define NOMBRE_ARCHIVO_CONSOLA  "Consola_fs.txt"
#define NOMBRE_ARCHIVO_LOG 		"fs.log"
#define MAXLINEA				4096			//Maximo de linea de configuracion
#define MAXNODOS				5				//Tamanio maximo del nombre de nodo
#define TAMANIO_BLOQUE			20480			//Tamanio maximo de bloque
#define MAXDIRECTORIOS			1024			//Cantidad maxima de directorios
#define TAMANIO_IP				16				//un string ejempl 192.168.001.123
#define	ES_MARTA				4				//emisor de mensaje Marta
#define ES_NODO					3				//emisor de mensaje Nodo
#define COMANDO					8				//comando(extra) para ver algo por pantalla
#define COMANDOBLOQUES			9				//idem el de arriba
#define CONSULTA_ARCHIVO		1				//marta consulta por un archivo

//Tamaño del buffer
#define BUFFERSIZE 10

typedef enum {
	CantidadArgumentosIncorrecta,
	//NoEsUnVolumen,
	NoSePudoAbrirConfig,
	NoSePuedeObtenerPuerto,
	NoSePuedeObtenerNodos,
	//ErrorEnLectura,
	//ErrorEnEscritura,
	OtroError,
} error;

//Lista de Nodos
t_list *lista_nodos;

//Lista de Archivos
t_list *lista_archivos;

// TIPOS //
typedef struct {
	char * nombre;
	char * ip;
	char * puerto;
	int estado;
} t_nodo;

static t_nodo *nodo_create(char *nombreNodo, char *ipNodo, char* puertoNodo, int activo) {
	t_nodo *new = malloc(sizeof(t_nodo));
	new->nombre = strdup(nombreNodo);
	new->ip = strdup(ipNodo);
	new->puerto = puertoNodo;
	new->estado = 1;
	return new;
}

struct configuracion {
	int puerto_listen;
	int cantidadNodos;
};

typedef struct {
	char nodo[80];
	int  nro_bloque;	
} t_block;

typedef struct {
	t_block bloques[3];
} t_bloques;

struct t_archivo {
	char nombre_archivo[255];	//Nombre del archivo
	int padre;					//Directorio Padre
	int estado;
	t_bloques *bloques;
};

struct configuracion configuracion;				//Datos de configuracion

void mostrarAyuda();								//Mostrar ayuda
void mostrarError(error unError);					//Mostrar error
void ejecutarComando(int);							//Ejecutar un comando
void formatearMDFS();								//Formatear el MDFS
void procesarArchivo();								//Procesar archivo
void procesarDirectorio();							//Procesar directorio
void copiarLocalAlMFDS(); 							//Copiar archivo local al MDFS
void copiarMDFSalFilesystem();						//Copiar archivo del MDFS al Filesystem
void solicitarMD5();								//Solicitar MD5 de un archivo
void procesarBloques();								//Procesar bloques
int agregarNodo();									//Agregar un nodo
void eliminarNodo();								//Eliminar un nodo
void comandoDesconocido(); 							//Comando desconocido

int leer_config();									//Leer archivo de configuracion
int conectar_marta();								//Conectar a marta
int operaciones_consola();							//Operaciones de la consola
void iniciar_mongo();								//Inicar Mongo DB		
void Comenzar_Consola();							//Manejo de la consola
int conectarNodo(int*,char*,char*);					//Conexion de Nodo
void HiloOrquestadorDeConexiones();					// maneja las conexiones entrantes
void implementoMarta(int*,char*,int*,char**,int);   //maneja las peticiones de Marta
void AtiendeMarta(char*,int*);						//maneja la consulta de archivos de marta
int ObtenerComandoMSJ(char*);							//Obtiene el tipo del comando del emisor
// METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...);
char* RecibirDatos(int,char*,int*,int*,int*);

// Logger del commons
t_log* logger;

// Archivo donde descargar info impresa por consola
FILE * g_ArchivoConsola;

//Mensaje de error global.
char* g_MensajeError;

// Definimos los hilos principales
pthread_t hOrquestadorConexiones, hConsola;

// - Bandera que controla la ejecución o no del programa. Si está en 0 el programa se cierra.
int g_Ejecutando = 1;

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

static t_bloque *bloque_create(char *bloque, t_array_copias *array) {
	t_bloque *new = malloc(sizeof(t_bloque));
	new->bloque = strdup(bloque);
	//new->estado = "procesado";
	new->array[0] = array[0];
	new->array[1] = array[1];
	new->array[2] = array[2];
	new->next=NULL;
	return new;
}

//Estructura Lista de Archivos
typedef struct{
    char *nombreArchivo;
    t_list *listaBloques;
}t_archivo;

static t_archivo *archivo_create(char *nombreArchivo) {
	t_archivo *new = malloc(sizeof(t_archivo));
	new->nombreArchivo = strdup(nombreArchivo);
	new->listaBloques= list_create();
	return new;
}
