/*
 * consola.h
 *
 *  Created on: 28/04/2015
 *      Author: gabriel
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>


typedef enum{
	CantidadArgumentosIncorrecta,
	NoEsUnVolumen,
	NoSePudoAbrirIn,
	NoSePudoAbrirOut1,
	NoSePudoAbrirOut2,
	ErrorEnLectura,
	ErrorEnEscritura,
	OtroError,
} Error;


#define IP 						"127.0.0.1"
#define PUERTO					"6667"
#define PACKAGESIZE				1024
#define CANTMAX					1			//Cantidad maxima para seleccionar un comando

// IPv4 AF_INET sockets:
struct sockaddr_in {
    short            sin_family;
    unsigned short   sin_port;
    struct in_addr   sin_addr;
    char             sin_zero[8];
};

struct in_addr {
    unsigned long s_addr;
};

struct sockaddr {
    unsigned short    sa_family;
    char              sa_data[14];
};



int crear_socket(int *);							//Crear un socket
int conectar_socket(int *, struct sockaddr_in*);	//Conectar un socket
void mostrarAyuda();								//Mostrar ayuda
void mostrarError(Error);							//Mostrar error
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

#endif /* CONSOLA_H_ */
