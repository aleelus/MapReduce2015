//#include "filesystem.h"


/*int main(int argc, char *argv[]){

	int sock_desc;
	struct sockaddr_in sock_addr;
	char* input;
	int eleccion;


	// Verifica sin argumentos

	if (argc == 1){
		mostrarAyuda();
		return EXIT_SUCCESS;
		}


	// Verifica cantidad de argumentos
	if (argc != 2){
		mostrarError(CantidadArgumentosIncorrecta);
		return EXIT_FAILURE;
		}

	// Reservo memoria para la seleccion
	if ((input = malloc(CANTMAX)) == NULL)
		return EXIT_FAILURE;


	// Abrir conexion con Marta
//	crear_conexion(sock_desc);
//	conectar_socket(sock_desc, sock_addr);

	while(eleccion != 10){
	// Solicitar opcion de comando
	printf("Ejecute una opcion: ");

//	if (fgets(input, CANTMAX, stdin) == 0)
//		printf("Error al seleccionar comando\n");
	scanf("%d", &eleccion);
	ejecutarComando(eleccion);
	}
//	if(strcmp(input, "\n") == 1){
//		eleccion = atoi(input);
//		ejecutarComando(eleccion);
	// Ejecutar

//	}


}*/
