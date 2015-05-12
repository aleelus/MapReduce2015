/*
 * consola_dev.c
 *
 *  Created on: 28/04/2015
 *      Author: gabriel
 */

#include "filesystem.h"
#include "sockets.h"

int eleccion;
/*int socket_desc , new_socket , c;
    struct sockaddr_in server , client;
    char *message, server_reply[2000];
    char buffer[SOCKET_MAX_BUFFER];
*/

//Mostrar Ayuda
void mostrarAyuda(){
	printf("Comandos posibles: \n");
	printf("1 - Formatear MDFS\n");
	printf("2 - Eliminar/Renombrar/Mover Archivos\n");
	printf("3 - Crear/Eliminar/Renombrar/Mover Directorios\n");
	printf("4 - Copiar un archivo local al MDFS\n");
	printf("5 - Copiar un archivo del MDFS al filesystem local\n");
	printf("6 - Solicitar el MD5 de un archivo en MDFS\n");
	printf("7 - Ver/Borrar/Copiar los bloques que componen un archivo\n");
	printf("8 - Agregar un nodo de datos\n");
	printf("9 - Eliminar un nodo de datos\n");
	printf("10 - Salir\n");
	printf("------------------------------------------------------------\n");
	printf("Ejecute una opcion: ");
	scanf("%d", &eleccion);
	ejecutarComando(eleccion);
}

//Mostrar Error
void mostrarError(error unError){
	switch(unError){
		case CantidadArgumentosIncorrecta: puts("CantidadArgumentosIncorrecta.");break;
		case NoSePudoAbrirConfig:		   puts("No se pudo abrir configuracion");break;
		case NoSePuedeObtenerPuerto:	   puts("No se pudo obtener puerto");break;
		case NoSePuedeObtenerNodos:		   puts("No se pudo obtener nodos");break;
		case OtroError:	 				   puts("OtroError.");break;
	};
}


//Ejecutar un comando
void ejecutarComando(int unComando){
	switch(unComando){
		case 1:	{ formatearMDFS(); break;}
		case 2: { procesarArchivo(); break;}
		case 3: { procesarDirectorio(); break;}
		case 4: { copiarLocalAlMFDS(); break;}
		case 5: { copiarMDFSalFilesystem(); break;}
		case 6: { solicitarMD5(); break;}
		case 7: { procesarBloques(); break;}
		case 8: { agregarNodo(); break;}
		case 9: { eliminarNodo(); break;}
		default: { comandoDesconocido(); break;}
	}
}

//Formatear el MDFS
void formatearMDFS(){
	printf("Esta opcion debe formatear el MDFS\n");
};

//Procesar archivo
void procesarArchivo(){
	printf("Esta opcion debe procesar un archivo\n");
};

//Procesar directorio
void procesarDirectorio(){
	printf("Esta opcion debe procesar un directorio\n");
};

//Copiar archivo local al MDFS
void copiarLocalAlMFDS(){
	printf("Esta opcion debe copiar un archivo local al MDFS\n");
};

//Copiar archivo del MDFS al Filesystem
void copiarMDFSalFilesystem(){
	printf("Esta opcion debe copiar un archivo del MDFS al Filesystem\n");
};

//Solicitar MD5 de un archivo
void solicitarMD5(){
	printf("Esta opcion debe solicitar el MD5 de un archivo\n");
};

//Procesar bloques
void procesarBloques(){
	printf("Esta opcion debe procesar bloques\n");
};

//Agregar un nodo
void agregarNodo(){
	printf("Esta opcion debe agregar un nodo\n");
};

//Eliminar un nodo
void eliminarNodo(){
	printf("Esta opcion debe eliminar un nodo\n");
};

//Comando desconocido
void comandoDesconocido(){
	printf("Esta opcion debe solicitar que ingrese un comando valido\n");
};

int leer_config(){
	
	char *lista_nodos;								//Lista de Nodos necesarios para empezar
	char linea[MAXLINEA];							//Linea de configuracion
	struct configuracion configuracion;				//Datos de configuracion
	nodos nodos[MAXNODOS];							//Datos de nodos
	FILE *config;									//Archivo de configuracion
	char *origen;									//Para manejo de cadenas
	char ch;										//Exclusion de lineas
	int i;

	if ( (config = fopen(PATH_CONFIG,"rt")) == NULL ) {
		mostrarError(NoSePudoAbrirConfig);
		return EXIT_FAILURE;
		}

	if ((lista_nodos = malloc(MAXLINEA+1)) == NULL)
		return EXIT_FAILURE;

	while(fgets(linea, MAXLINEA, config) != NULL){		// Obtener valores de configuracion

		if(sscanf(linea, "%*[^\n#]%c", &ch) != 1){
			;											//Se ignoran las lineas en blanco y comentarios
		} 

		if(sscanf(linea, " PUERTO_LISTEN= %u", &configuracion.puerto_listen) == 1){
			printf("%u\n", configuracion.puerto_listen);
		} 

		if(sscanf(linea, " LISTA_NODOS=[%s]", lista_nodos) == 1){
			for(i = 0, origen = strtok(lista_nodos, ","); origen; i++, origen = strtok(NULL, ",")){
				sscanf(origen, "Nodo%c", &nodos[i].nodo);
				printf("Nodos: %c\n", nodos[i].nodo);
			}
			configuracion.lista_nodos 	= nodos;
			configuracion.cantidadNodos = i;
		}
	}
	fclose(config);
	return EXIT_SUCCESS;

}

//Conectar a Marta
int conectar_marta(){
    //Create socket
    /*
    if ((socket_desc = socket(AF_INET , SOCK_STREAM , 0)) == -1)
    	return EXIT_FAILURE;

    //Prepare the sockaddr_in structure
    server.sin_family 	   = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port 	   = htons( 8888 );

    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
        return EXIT_FAILURE;

    //Send some data
    message = "GET / HTTP/1.1\r\n\r\n";
    if( send(socket_desc , message , strlen(message) , 0) < 0)
        return EXIT_FAILURE;

    //Receive a reply from the server
    if( recv(socket_desc, server_reply , 2000 , 0) < 0)
    	return EXIT_FAILURE;

    puts("Reply received\n");
    puts(server_reply);    
    */
	return EXIT_SUCCESS;

}

//Conectar nodos
int conectar_nodos(){

    t_socket_server *server = sockets_create_server(INADDR_ANY, 8888);
    t_socket_client *client;
    t_socket_buffer *buffer;
	char *message;

    sockets_listen(server);
    puts("Esperando conexiones de nodos...");
    
    while( client = sockets_accept(server)){
    	puts("Nodo conectado");

 		//Reply to the client
        message = "El nodo se conecto al filesystem..\n";
        sockets_write(client, message, strlen(message));

        buffer = sockets_recv(client);
        printf("%s\n", buffer->data);		

    };

    sockets_buffer_destroy(buffer);

    sockets_destroy_client(client);

    sockets_destroy_server(server);

    return EXIT_SUCCESS;


}

void LevantarConfig() {
	int puerto_listen;
	char *lista_nodos;

	t_config* config = config_create(PATH_CONFIG);
	// Nos fijamos si el archivo de conf. pudo ser leido y si tiene los parametros
	if (config->properties->table_current_size != 0) {

		// Puerto de escucha
		if (config_has_property(config, "PUERTO_LISTEN")) {
//			g_Puerto_Listen = config_get_int_value(config, "PUERTO_LISTEN");
			puerto_listen = config_get_int_value(config, "PUERTO_LISTEN");
			printf("Puerto Listen: %d", puerto_listen);
		} else
			//Error("No se pudo leer el parametro PUERTO_LISTEN");
			printf("No se pudo leer el parametro PUERTO_LISTEN");

		// Lista de nodos minima que deben estar conectados para que funcione
		if (config_has_property(config, "LISTA_NODOS")) {
			//g_Lista_Nodos = config_get_string_value(config,"LISTA_NODOS");
			lista_nodos = config_get_string_value(config,"LISTA_NODOS");
			printf("Lista Nodos: %s", lista_nodos);
		} else
//			//Error("No se pudo leer el parametro LISTA_NODOS");
			printf("No se pudo leer el parametro LISTA_NODOS");

	} else {
		//ErrorFatal("No se pudo abrir el archivo de configuracion");
	}
	if (config != NULL ) {
		free(config);
	}
}

void iniciar_mongo(){
	
}
