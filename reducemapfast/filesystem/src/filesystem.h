// Bibliotecas //
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


#define IP 						"127.0.0.1"
#define PUERTO					"6667"
#define PACKAGESIZE				1024
#define CANTMAX					1			//Cantidad maxima para seleccionar un comando
#define PATH_CONFIG 			"/home/gabriel/workspace/sisop/tp-2015-1c-los-barderos/reducemapfast/filesystem/src/config.cfg"	//Ruta del config
#define NOMBRE_ARCHIVO_CONSOLA  "Archivo_msp.txt"
#define NOMBRE_ARCHIVO_LOG 		"msp.log"
#define MAXLINEA				4096			//Maximo de linea de configuracion
#define MAXNODOS				5				//Tamanio maximo del nombre de nodo
#define TAMANIO_BLOQUE			20480			//Tamanio maximo de bloque
#define MAXDIRECTORIOS			1024			//Cantidad maxima de directorios


typedef enum {
	CantidadArgumentosIncorrecta,
	NoEsUnVolumen,
	NoSePudoAbrirConfig,
	NoSePuedeObtenerPuerto,
	NoSePuedeObtenerNodos,
	ErrorEnLectura,
	ErrorEnEscritura,
	OtroError,
} error;

// TIPOS //
typedef struct {
	char nodo;
} nodos;

struct configuracion {
	unsigned int puerto_listen;
	nodos* lista_nodos;
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
void agregarNodo();									//Agregar un nodo
void eliminarNodo();								//Eliminar un nodo
void comandoDesconocido(); 							//Comando desconocido

int leer_config();									//Leer archivo de configuracion
int conectar_nodos();								//Conectar nodos
int conectar_marta();								//Conectar a marta

void iniciar_mongo();								//Inicar Mongo DB		

// VARIABLES GLOBALES //



// METODOS CONFIGURACION //
//void LevantarConfig();

// METODOS MANEJO DE ERRORES //
//void Error(const char* mensaje, ...);

// Logger del commons
//t_log* logger;



