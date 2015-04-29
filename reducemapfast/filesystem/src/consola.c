#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "consola.h"

int main(int argc, char *argv[]){

	int sock_desc;
	struct sockaddr_in sock_addr;

	/* Verifica sin argumentos */
	if (argc == 1){
		mostrarAyuda();
		return EXIT_SUCCESS;
		}


	/* Verifica cantidad de argumentos */
	if (argc != 5){
		mostrarError(CantidadArgumentosIncorrecta);
		return EXIT_FAILURE;
		}


	// Abrir conexion con Marta
	crear_conexion(sock_desc);
	conectar_socket(sock_desc, sock_addr);

	// Solicitar opcion de comando

	// Ejecutar

}
