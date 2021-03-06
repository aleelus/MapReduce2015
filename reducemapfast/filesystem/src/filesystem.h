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
#include <bson.h>
#include <mongoc.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define SUBIRARCHIVO			3
#define COLOR_VERDE   			"\x1b[32m"
#define DEFAULT   				"\x1b[0m"
#define CONEXION				1
#define PATH_CONFIG 			"config.cfg"		//Ruta del config
#define NOMBRE_ARCHIVO_CONSOLA  "Consola_fs.txt"	//Nombre de archivo de consola
#define NOMBRE_ARCHIVO_LOG 		"fs.log"			//Nombre de archivo de log
#define MAXLINEA				4096				//Maximo de linea de configuracion
#define MAXNODOS				5					//Tamanio maximo del nombre de nodo
#define TAMANIO_BLOQUE			20971520			//Tamanio maximo de bloque
#define MAXDIRECTORIOS			1024				//Cantidad maxima de directorios
#define TAMANIO_IP				16					//un string ejempl 192.168.001.123
#define	ES_MARTA				4					//emisor de mensaje Marta
#define ES_NODO					3					//emisor de mensaje Nodo
#define COMANDO					8					//comando(extra) para ver algo por pantalla
#define COMANDO2				7
#define COMANDOFILESYSTEM		9					//idem el de arriba
#define CONSULTA_ARCHIVO		1					//marta consulta por un archivo
#define BUFFERSIZE 				50					//Tamaño del buffer


sem_t semLNodos,semHilos,semArchivos,semTamanioTotal,semLFs;

// TIPOS //
typedef enum {
	CantidadArgumentosIncorrecta,
	NoSePudoAbrirConfig,
	NoSePuedeObtenerPuerto,
	NoSePuedeObtenerNodos,
	NosePuedeCrearHilo,
	OtroError,
} t_error;							//Tipo error

typedef struct {

 char *nombre;
 char *bloqueArchivo;
 char *nombreArchivo;
 int padre;

}t_array_nodo;

typedef struct {
	char * nombre;
	char * ip;
	char * puerto;
	char * tamanio;
	int estado;
	t_list* bloquesDisponibles;
} t_nodo;							//Tipo nodo

typedef struct {
	int bloque;
	int vacio;
}t_bloque_disponible;

typedef struct {
	char *nombreNodo;
	char *nro_bloque;
	int activo;
} t_array_copias;							//Tipo Bloque


typedef struct {
	int bloque;
	t_list *listaCopias;
} t_bloque;						//Tipo Bloques

typedef struct{
    char *nombreArchivo;
    int padre;			//directorio padre
    long unsigned tamanio;
    int estado; 			//0: no disponible 1: disponible
    t_list *listaBloques;
}t_archivo;						//Tipo Archivo


typedef struct{
    int index;
    char *directorio;
    int padre;
}t_filesystem;						//Tipo de estructura de File System

typedef struct{
	char* buffer;
	char* ip;
	char* puerto;
	int bloque;
} t_envio_nodo;


//Variables globales
/*********************/
t_log* logger;								// Logger del commons
t_list *lista_nodos;						//Lista de Nodos
char * nombre;
t_list *lista_archivos;						//Lista de Archivos
FILE* g_ArchivoConsola;						// Archivo donde descargar info impresa por consola
char* g_MensajeError;						//Mensaje de error global.
pthread_t hOrquestadorConexiones, hConsola, hNodos;	// Definimos los hilos principales
t_list *lista_filesystem;					//Lista para la estructura del filesystem
int fs_Puerto;
int fs_CantidadNodos;
t_archivo * archivo;
int nroBloque;
t_list ** arrayNodos;
int indexGlobal;
char letra;


//FUNCIONES
long unsigned tamanioDisponible();
int validarDirectorio(char *directorio,int i);
void mostrarAyuda();								//Mostrar ayuda
void mostrarError(t_error unError);					//Mostrar error
void ejecutarComando(int);							//Ejecutar un comando
void formatearMDFS();								//Formatear el MDFS
int procesarArchivo();								//Procesar archivo
void procesarDirectorio();							//Procesar directorio
void copiarLocalAlMFDS(); 							//Copiar archivo local al MDFS
void copiarMDFSalFilesystem();						//Copiar archivo del MDFS al Filesystem
void solicitarMD5();								//Solicitar MD5 de un archivo
void procesarBloques();								//Procesar bloques
int  agregarNodo();									//Agregar un nodo
void eliminarNodo();								//Eliminar un nodo
void comandoDesconocido(); 							//Comando desconocido
long unsigned ObtenerLu (char *buffer);

void levantarConfig();									//Leer archivo de configuracion
int  conectar_marta();								//Conectar a marta
int  operaciones_consola();							//Operaciones de la consola
void iniciar_mongo();								//Inicar Mongo DB		
void Comenzar_Consola();							//Manejo de la consola
int  conectarNodo(int*,char*,char*);					//Conexion de Nodo
void HiloOrquestadorDeConexiones();					// maneja las conexiones entrantes
void implementoMarta(int*,char*,int*,char**,int);   //maneja las peticiones de Marta
int  ObtenerComandoMSJ(char*);						//Obtiene el tipo del comando del emisor
int enviarBloque(char *bufferAux);

void  Error(const char* mensaje, ...);				//Generar error
char* RecibirDatos(int,char*,int*,int*,int*);		//Recibir datos
int   atiendeNodo(char*,int*);						//Atiende un nodo


int   ChartToInt(char x);
int   PosicionDeBufferAInt(char* buffer, int posicion);
int   ObtenerTamanio (char *buffer , int posicion, int dig_tamanio);
char* DigitosNombreArchivo(char *buffer,int *posicion);
int   AtiendeNodo(char* buffer,int*cantRafaga);
int   agregarNodo();
int  AtiendeMarta(char* buffer,int*cantRafaga,char**bufferE);
int   ObtenerComandoMSJ(char* buffer);
int   cuentaDigitos(int valor);
int   EnviarInfoMarta(int socket,char* buffer);
void  implementoMarta(int *id,char * buffer,int * cantRafaga,char ** mensaje, int socket);
void  implementoNodo(char * buffer,int * cantRafaga,char ** mensaje, int socket);
char* RecibirDatos(int socket, char *buffer, int *bytesRecibidos,int *cantRafaga,int *tamanio);
int   EnviarDatos(int socket, char *buffer, int cantidadDeBytesAEnviar);
void  CerrarSocket(int socket);
void  RecorrerNodos();
int   AtiendeCliente(void * arg);
void  ErrorFatal(const char* mensaje, ...);
int   conectarNodo(int * socket_Nodo, char* ipNodo, char* puertoNodo);
int   corte_consola();
void  Comenzar_Consola() ;
void  Error(const char* mensaje, ...);
void  RecorrerListaNodos();
int   operaciones_consola();
t_bloque *bloque_create(int bloque);
t_archivo *archivo_create(char*,long unsigned,int,int);
t_nodo *nodo_create(char *nombreNodo, char *ipNodo, char* puertoNodo, char* tamanio, int activo);
t_nodo* buscarNodo(char*,char*);
t_filesystem* filesystem_create(int,char*,int);
void bloque_destroy(t_bloque*);
void filesystem_destroy(t_filesystem*);
void nodo_destroy(t_nodo*);
void archivo_destroy(t_archivo*);
t_array_nodo *array_nodo_create(char *);
t_envio_nodo *envio_nodo_create(char *,char*,char*,int);
t_array_copias* array_copias_create(char* nombre, char* bloque);
t_bloque_disponible *bloque_disponible_create(int);
int sendall(int s, char *buf, long unsigned *len);
int buscarNodoEnArrayPorNombre(char *);
char* obtenerSubBuffer(char*);
t_nodo* buscarNodoPorNombre(char* nombre);
t_archivo* buscarArchivoPorNombre(char* nombre,int padre);
void eliminarFilesystem();
void eliminarArchivos();
int armarArchivoPorHilo();
int armarArchivoMD5PorHilo();
int armarArchivo();
int armarArchivoMD5();
int cargarArchivo(int *padre,FILE ** fArchivo,char**nombreArchivo);
int cargarArchivoMD5(int *padre,FILE ** fArchivo,char**nombreArchivo);
int recuperarArchivo(FILE *fArchivo,int padre, char*nombre);
int recuperarArchivoMD5(FILE *fArchivo,int padre, char*nombre);


void mongo_db_open();
void mongo_db_close();

bson_t* armar_nodo_mongo(t_nodo* el_nodo,bson_t*bloquesDisp);
bson_t* armar_nodo_mongo2(t_nodo* el_nodo,bson_t*bloquesDisp);

bool crear_nodo_mongo(t_nodo* el_nodo);
bool borrar_nodo_mongo(t_nodo* el_nodo);
int grabar_nodo_mongo(t_nodo* el_nodo_viejo, t_nodo* el_nodo_nuevo);
int leer_nodo_mongo();
bson_t* armar_filesystem_mongo(t_filesystem* el_fs);
bool crear_filesystem_mongo(t_filesystem* el_fs);
bool borrar_filesystem_mongo(t_filesystem* el_fs);
int grabar_filesystem_mongo(t_filesystem* el_fs_viejo, t_filesystem* el_fs_nuevo);
int leer_filesystem_mongo();
bson_t* armar_archivo_mongo(t_archivo*el_archivo, bson_t *listaBloques, bson_t *copiasArray);
bool crear_archivo_mongo(t_archivo* el_archivo);
bool borrar_archivo_mongo(t_archivo* el_archivo);
int grabar_archivo_mongo(t_archivo* el_archivo_viejo, t_archivo* el_archivo_nuevo);
int leer_archivo_mongo();
void cargar_listas_mongo();
int manejoDeBloques();
int verBloquesArchivo();
void RecorrerListaBloques();
