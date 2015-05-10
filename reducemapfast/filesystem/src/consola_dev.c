/*
 * consola_dev.c
 *
 *  Created on: 28/04/2015
 *      Author: gabriel
 */

#include "consola.h"

int eleccion;
int socket_desc , new_socket , c;
    struct sockaddr_in server , client;
    char *message;

int crear_socket(int socket_desc){
	if ((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

int conectar_socket(int socket_desc, struct sockaddr_in server){
	server.sin_addr.s_addr = inet_addr("74.125.235.20");
	server.sin_family = AF_INET;
	server.sin_port = htons( 80 );

	//Connect to remote server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0){
		puts("Conection Error!");
		return EXIT_FAILURE;
	}

    puts("Connected");
    return EXIT_SUCCESS;
}

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

//Conectar nodos
int conectar_nodos(){
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("bind failed");
    }
    puts("bind done");

    //Listen
    listen(socket_desc , 3);


    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");

        //Reply to the client
        message = "Hello Client , I have received your connection. But I have to go now, bye\n";
        write(new_socket , message , strlen(message));
    }

    if (new_socket<0)
    {
        perror("accept failed");
        return 1;
    }

    return 0;
}
