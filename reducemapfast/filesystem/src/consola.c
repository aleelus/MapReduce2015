
#include <stdlib.h>
#include "consola.h"

int main(int argc, char *argv[]){

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

	// Solicitar opcion de comando

	// Ejecutar

}
