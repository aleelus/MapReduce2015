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


#define IP 						"127.0.0.1"
#define PUERTO					"6667"
#define PACKAGESIZE				1024
#define CANTMAX					1			//Cantidad maxima para seleccionar un comando


int crear_socket(int);							//Crear un socket
int conectar_socket(int, struct sockaddr_in);		//Conectar un socket
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

#endif /* CONSOLA_H_ */
